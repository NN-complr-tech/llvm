#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

namespace {
class NullPtrDereferencePass : public MachineFunctionPass {
public:
  static char ID;
  NullPtrDereferencePass() : MachineFunctionPass(ID) {}
  bool runOnMachineFunction(MachineFunction &MF) override;
};

char NullPtrDereferencePass::ID = 0;

bool NullPtrDereferencePass::runOnMachineFunction(MachineFunction &func) {
  llvm::outs() << func.getName() << '\n';
  return true;
}
} // namespace

static RegisterPass<NullPtrDereferencePass> X("levonychev-nullptr-x86", "description pass", false,
                                   false);
