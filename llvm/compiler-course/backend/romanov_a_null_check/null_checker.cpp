#include "MCTargetDesc/X86BaseInfo.h"
#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

namespace {
class NullCheckerPass : public MachineFunctionPass {
public:
  static char ID;
  NullCheckerPass() : MachineFunctionPass(ID) {}
  bool runOnMachineFunction(MachineFunction &MF) override;
};

char NullCheckerPass::ID = 0;

bool NullCheckerPass::runOnMachineFunction(MachineFunction &MF) {
  bool Modified = false;
  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();

  for (MachineBasicBlock &MBB : MF) {
    for (MachineBasicBlock::iterator It = MBB.begin(); It != MBB.end(); ++It) {
      MachineInstr &MI = *It;

      if (!MI.mayLoad() && !MI.mayStore()) {
        continue;
      }

      const MCInstrDesc &Desc = MI.getDesc();
      int MemIdx = X86II::getMemoryOperandNo(Desc.TSFlags);
      if (MemIdx < 0) {
        continue;
      }
      MemIdx += X86II::getOperandBias(Desc);

      const MachineOperand &BaseOp = MI.getOperand(MemIdx + X86::AddrBaseReg);
      if (!BaseOp.isReg()) {
        continue;
      }

      Register Base = BaseOp.getReg();
      if (!Base.isValid()) {
        continue;
      }

      if (Base == X86::RSP || Base == X86::RBP || Base == X86::RIP) {
        continue;
      }

      DebugLoc DL = MI.getDebugLoc();
      BuildMI(MBB, It, DL, TII->get(TargetOpcode::COPY), X86::RDI).addReg(Base);
      BuildMI(MBB, It, DL, TII->get(X86::CALL64pcrel32))
          .addExternalSymbol("check_null");
      Modified = true;
    }
  }

  return Modified;
}
} // namespace

static RegisterPass<NullCheckerPass> X("null-checker-x86",
                                       "Insert null checks before dereference",
                                       false, false);