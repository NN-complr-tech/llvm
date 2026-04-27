#include "X86.h"
#include "X86InstrInfo.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/Function.h"
#include <map>

using namespace llvm;

namespace {

class RysevInlining : public MachineFunctionPass {
public:
  static char ID;
  RysevInlining() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<MachineModuleInfoWrapperPass>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

private:
  static const unsigned MAX_INSTR = 15;
  static const unsigned MAX_REC_DEPTH = 3;

  std::map<const Function *, unsigned> recursionDepth;

  bool canBeInlined(const MachineFunction &MF) const;
  bool performInlining(MachineFunction &caller, MachineBasicBlock &block,
                       MachineInstr &callMI, unsigned curDepth);
};

char RysevInlining::ID = 0;

bool RysevInlining::canBeInlined(const MachineFunction &MF) const {
  if (MF.size() != 1)
    return false;

  unsigned cnt = 0;
  for (const MachineBasicBlock &BB : MF) {
    for (const MachineInstr &MI : BB) {
      if (!MI.isDebugInstr()) {
        ++cnt;
        if (cnt > MAX_INSTR)
          return false;
      }
    }
  }
  return true;
}

bool RysevInlining::performInlining(MachineFunction &caller,
                                    MachineBasicBlock &block,
                                    MachineInstr &callMI, unsigned curDepth) {
  if (callMI.getNumOperands() == 0)
    return false;

  MachineOperand &op0 = callMI.getOperand(0);
  if (!op0.isGlobal())
    return false;

  if (callMI.getOpcode() != X86::CALL64pcrel32)
    return false;

  const Function *targetFn = dyn_cast<Function>(op0.getGlobal());
  if (!targetFn)
    return false;

  if (recursionDepth[targetFn] >= MAX_REC_DEPTH)
    return false;

  bool isRecursive = (targetFn == &caller.getFunction());

  MachineFunction *calleeMF = nullptr;
  if (isRecursive) {
    calleeMF = &caller;
  } else {
    auto &MMI = getAnalysis<MachineModuleInfoWrapperPass>().getMMI();
    calleeMF = MMI.getMachineFunction(*targetFn);
    if (!calleeMF)
      return false;
  }

  if (!canBeInlined(*calleeMF))
    return false;

  recursionDepth[targetFn]++;

  MachineRegisterInfo &callerMRI = caller.getRegInfo();
  MachineBasicBlock &calleeEntry = calleeMF->front();

  std::map<Register, Register> regMap;
  SmallVector<MachineInstr *, 16> instrsToClone;

  for (MachineInstr &mi : calleeEntry) {
    if (!mi.isReturn())
      instrsToClone.push_back(&mi);
  }

  for (MachineInstr *origMI : instrsToClone) {
    MachineInstr *clonedMI = caller.CloneMachineInstr(origMI);

    for (MachineOperand &mo : clonedMI->operands()) {
      if (!mo.isReg())
        continue;
      Register oldReg = mo.getReg();
      if (!oldReg.isVirtual())
        continue;

      auto it = regMap.find(oldReg);
      if (it == regMap.end()) {
        const TargetRegisterClass *rc =
            calleeMF->getRegInfo().getRegClass(oldReg);
        Register newReg = callerMRI.createVirtualRegister(rc);
        it = regMap.insert({oldReg, newReg}).first;
      }
      mo.setReg(it->second);
    }

    block.insert(callMI.getIterator(), clonedMI);
  }

  callMI.eraseFromParent();

  if (!isRecursive)
    recursionDepth[targetFn]--;

  return true;
}

bool RysevInlining::runOnMachineFunction(MachineFunction &MF) {
  bool changed = false;
  bool iterChanged = true;

  while (iterChanged) {
    iterChanged = false;

    for (MachineBasicBlock &MBB : MF) {
      for (auto it = MBB.begin(); it != MBB.end();) {
        MachineInstr &mi = *it;
        ++it;
        if (performInlining(MF, MBB, mi, 0)) {
          iterChanged = true;
          changed = true;
        }
      }
    }
  }

  return changed;
}

} // namespace

static RegisterPass<RysevInlining>
    X("example-x86-inline",
      "Rysev Inlining Pass (small funcs, recursion depth <=3)", false, false);