#include "X86.h"
#include "X86InstrInfo.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/Function.h"

#include <map>

using namespace llvm;

namespace {

class LukinInliningPass : public MachineFunctionPass {
  static const int maxCountOfInstructions = 15;
  static const int recursionMaxDepth = 3;

public:
  static char ID;
  LukinInliningPass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineModuleInfoWrapperPass>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

private:
  int getCountOfInstructions(MachineFunction &MF) const;
  bool Inline(MachineFunction &Caller, MachineBasicBlock &MBB, MachineInstr &MI,
              std::map<const Function *, int> &depths);
};

char LukinInliningPass::ID = 0;

bool LukinInliningPass::runOnMachineFunction(MachineFunction &MF) {
  std::map<const Function *, int> recursionDepths;
  bool changed = false;
  bool onIterChanged = true;

  while (onIterChanged) {
    onIterChanged = false;

    for (auto &MBB : MF) {
      for (auto it = MBB.begin(); it != MBB.end();) {
        MachineInstr &Ins = *it++;
        if (Ins.getOpcode() == X86::CALL64pcrel32) {
          onIterChanged |= Inline(MF, MBB, Ins, recursionDepths);
          changed |= onIterChanged;
        }
      }
    }
  }
  return changed;
}

int LukinInliningPass::getCountOfInstructions(MachineFunction &MF) const {
  int count = 0;

  for (const auto &MBB : MF) {
    for (const auto &MI : MBB) {
      if (!MI.isDebugInstr() && !MI.isMetaInstruction()) {
        count++;
      }
    }
  }
  return count;
}

bool LukinInliningPass::Inline(MachineFunction &Caller, MachineBasicBlock &MBB,
                               MachineInstr &Ins,
                               std::map<const Function *, int> &depths) {
  if (Ins.getNumOperands() == 0)
    return false;
  MachineOperand &operand = Ins.getOperand(0);
  if (!operand.isGlobal())
    return false;

  const Function *CalleeF = dyn_cast<Function>(operand.getGlobal());
  if (!CalleeF)
    return false;
  if (depths[CalleeF] >= recursionMaxDepth)
    return false;

  MachineFunction *CalleeMF = nullptr;
  Function *CallerF = &Caller.getFunction();
  if (CalleeF == CallerF) {
    CalleeMF = &Caller;
  } else {
    auto &MMI = getAnalysis<MachineModuleInfoWrapperPass>().getMMI();
    CalleeMF = MMI.getMachineFunction(*CalleeF);
    if (!CalleeMF) {
      return false;
    }
  }

  int insCount = LukinInliningPass::getCountOfInstructions(*CalleeMF);
  if (insCount > maxCountOfInstructions)
    return false;

  depths[CalleeF]++;

  MachineRegisterInfo &CallerMRI = Caller.getRegInfo();
  MachineRegisterInfo &CalleeMRI = CalleeMF->getRegInfo();

  std::map<Register, Register> regsTransform;
  SmallVector<MachineInstr *, LukinInliningPass::maxCountOfInstructions + 1>
      clone;

  for (auto &MInst : CalleeMF->front()) {
    if (!MInst.isReturn())
      clone.push_back(&MInst);
  }

  for (int i = 0; i < static_cast<int>(clone.size()); i++) {
    MachineInstr *OriginalInstr = clone[i];
    MachineInstr *InlinedInstr = Caller.CloneMachineInstr(OriginalInstr);

    for (int opIdx = 0, numOps = InlinedInstr->getNumOperands();
         opIdx != numOps; opIdx++) {
      MachineOperand &MOp = InlinedInstr->getOperand(opIdx);

      if (MOp.isReg() && MOp.getReg().isVirtual()) {
        Register OldVReg = MOp.getReg();

        auto Mapping = regsTransform.find(OldVReg);
        if (Mapping == regsTransform.end()) {
          const TargetRegisterClass *TClass = CalleeMRI.getRegClass(OldVReg);
          Register NewVReg = CallerMRI.createVirtualRegister(TClass);
          Mapping = regsTransform.insert({OldVReg, NewVReg}).first;
        }

        MOp.setReg(Mapping->second);
      }
    }

    MBB.insert(Ins, InlinedInstr);
  }
  Ins.eraseFromParent();

  return true;
}

} // namespace

static RegisterPass<LukinInliningPass>
    X("InliningPass", "function inlining pass -x86", false, false);