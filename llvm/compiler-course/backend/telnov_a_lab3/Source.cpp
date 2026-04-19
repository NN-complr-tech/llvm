#include "X86.h"
#include "X86InstrInfo.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineLoopInfo.h"

using namespace llvm;

namespace {

class ExamplePass : public MachineFunctionPass {
public:
  static char ID;
  ExamplePass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override {
    MachineLoopInfo &MLI = getAnalysis<MachineLoopInfoWrapperPass>().getLI();

    SmallVector<MachineLoop *, 8> OrderedLoops;
    for (MachineLoop *TopLevel : MLI)
      collectInPostOrder(TopLevel, OrderedLoops);

    bool Changed = false;
    for (MachineLoop *L : OrderedLoops)
      Changed |= tryUnrollLoop(MF, MLI, L);

    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    MachineFunctionPass::getAnalysisUsage(AU);
    AU.addRequired<MachineLoopInfoWrapperPass>();
    AU.setPreservesCFG();
  }

private:
  static constexpr int MaxAllowedIterations = 5;

  void collectInPostOrder(MachineLoop *L,
                          SmallVectorImpl<MachineLoop *> &Loops) const {
    for (MachineLoop *Inner : *L)
      collectInPostOrder(Inner, Loops);
    Loops.push_back(L);
  }

  bool tryUnrollLoop(MachineFunction &MF, MachineLoopInfo &MLI,
                     MachineLoop *L) const {
    MachineBasicBlock *Header = L->getHeader();
    MachineBasicBlock *Latch = L->getLoopLatch();
    if (!Header || !Latch)
      return false;

    if (!hasSinglePreheader(L))
      return false;

    MachineBasicBlock *ExitSource = findSingleExitSource(L);
    if (!ExitSource || ExitSource != Latch)
      return false;

    int TripCount = readConstantTripCount(*Latch);
    if (TripCount <= 1 || TripCount > MaxAllowedIterations)
      return false;

    SmallVector<MachineInstr *, 16> Payload;
    collectLoopPayload(L, MLI, Payload);
    if (Payload.empty())
      return false;

    auto InsertPos = Latch->getFirstTerminator();
    if (InsertPos == Latch->end())
      return false;

    for (int i = 1; i < TripCount; ++i) {
      for (MachineInstr *MI : Payload) {
        MachineInstr *Clone = MF.CloneMachineInstr(MI);
        Latch->insert(InsertPos, Clone);
      }
    }

    scaleInductionStep(*Latch, TripCount);
    return true;
  }

  bool hasSinglePreheader(MachineLoop *L) const {
    MachineBasicBlock *Header = L->getHeader();
    MachineBasicBlock *OutsidePred = nullptr;

    for (MachineBasicBlock *Pred : Header->predecessors()) {
      if (L->contains(Pred))
        continue;
      if (OutsidePred)
        return false;
      OutsidePred = Pred;
    }

    return OutsidePred != nullptr;
  }

  MachineBasicBlock *findSingleExitSource(MachineLoop *L) const {
    MachineBasicBlock *Result = nullptr;

    for (MachineBasicBlock *BB : L->blocks()) {
      for (MachineBasicBlock *Succ : BB->successors()) {
        if (L->contains(Succ))
          continue;
        if (Result && Result != BB)
          return nullptr;
        Result = BB;
      }
    }

    return Result;
  }

  int readConstantTripCount(MachineBasicBlock &Latch) const {
    for (MachineInstr &MI : Latch) {
      unsigned Opc = MI.getOpcode();

      if (Opc != X86::CMP32ri8 && Opc != X86::CMP32ri)
        continue;

      for (const MachineOperand &Op : MI.operands()) {
        if (Op.isImm())
          return (int)Op.getImm();
      }
    }
    return -1;
  }

  void collectLoopPayload(MachineLoop *L, MachineLoopInfo &MLI,
                          SmallVectorImpl<MachineInstr *> &Payload) const {
    for (MachineBasicBlock *BB : L->blocks()) {
      if (MLI.getLoopFor(BB) != L)
        continue;

      for (MachineInstr &MI : *BB) {
        if (MI.isBranch() || MI.isTerminator() || MI.isDebugInstr())
          continue;
        Payload.push_back(&MI);
      }
    }
  }

  void scaleInductionStep(MachineBasicBlock &Latch, int Step) const {
    for (MachineInstr &MI : Latch) {
      if (MI.getOpcode() != X86::ADD32ri8)
        continue;

      for (MachineOperand &Op : MI.operands()) {
        if (Op.isImm() && Op.getImm() == 1) {
          Op.setImm(Step);
          return;
        }
      }
    }
  }
};

char ExamplePass::ID = 0;

} // namespace

static RegisterPass<ExamplePass> X("example-x86", "loop unrolling pass", false,
                                   false);