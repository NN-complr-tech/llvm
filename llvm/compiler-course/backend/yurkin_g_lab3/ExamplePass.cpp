// ExamplePass.cpp
#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
class ExamplePass : public MachineFunctionPass {
public:
  static char ID;
  ExamplePass() : MachineFunctionPass(ID) {}
  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  /// Map INC/DEC opcode -> pair(isInc, width)
  /// width: 8,32,64 (we use it to pick appropriate ADD/SUB opcode)
  bool isIncOrDec(unsigned Opcode, bool &IsInc, unsigned &Width) {
    const char *Name = MF_OpcodeName(Opcode);
    if (!Name)
      return false;
    // Simple name-based detection. Adjust if your target uses different naming.
    if (strstr(Name, "INC")) {
      IsInc = true;
    } else if (strstr(Name, "DEC")) {
      IsInc = false;
    } else {
      return false;
    }

    // Determine width from opcode name
    if (strstr(Name, "8"))
      Width = 8;
    else if (strstr(Name, "64"))
      Width = 64;
    else
      Width = 32; // default to 32-bit if not explicit
    return true;
  }

  // Helper to get opcode name safely (wrap TargetInstrInfo::getName)
  const char *MF_OpcodeName(unsigned Opcode) {
    // We can't access TII here; this function will be replaced at runtime.
    // The pass will call TII->getName(opcode) directly where TII is available.
    return nullptr;
  }
};

char ExamplePass::ID = 0;

bool ExamplePass::runOnMachineFunction(MachineFunction &MF) {
  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
  MachineRegisterInfo &MRI = MF.getRegInfo();

  // Helper lambda to get opcode name via TII
  auto opcodeName = [&](unsigned Opc) -> const char * {
    return TII->getName(Opc);
  };

  // Mapping helper: choose ADD/SUB immediate opcode for given width
  auto selectAddSubOpcode = [&](bool IsAdd, unsigned Width) -> unsigned {
    // These opcode names are common in X86 backend; adjust if your target
    // differs.
    if (Width == 8) {
      return IsAdd ? X86::ADD8ri : X86::SUB8ri;
    } else if (Width == 64) {
      return IsAdd ? X86::ADD64ri32 : X86::SUB64ri32;
    } else { // 32-bit default
      return IsAdd ? X86::ADD32ri : X86::SUB32ri;
    }
  };

  bool Changed = false;

  for (MachineBasicBlock &MBB : MF) {
    for (auto MI = MBB.begin(); MI != MBB.end();) {
      MachineInstr &Instr = *MI;
      unsigned Opc = Instr.getOpcode();
      const char *Name = opcodeName(Opc);
      if (!Name) {
        ++MI;
        continue;
      }

      bool IsInc = false;
      unsigned Width = 0;
      if (!(strstr(Name, "INC") || strstr(Name, "DEC"))) {
        ++MI;
        continue;
      }
      IsInc = strstr(Name, "INC") != nullptr;
      // Determine width heuristically from opcode name
      if (strstr(Name, "8"))
        Width = 8;
      else if (strstr(Name, "64"))
        Width = 64;
      else
        Width = 32;

      // We only handle register-form INC/DEC here (e.g., INC32r, DEC64r).
      // If instruction has a memory operand or immediate, skip it.
      // Find destination register (first explicit def or first operand that is
      // a reg)
      unsigned Reg = 0;
      for (const MachineOperand &MO : Instr.operands()) {
        if (MO.isReg() && MO.isDef()) {
          Reg = MO.getReg();
          break;
        }
      }
      if (!Reg) {
        // try first reg use
        for (const MachineOperand &MO : Instr.operands()) {
          if (MO.isReg()) {
            Reg = MO.getReg();
            break;
          }
        }
      }
      if (!Reg) {
        ++MI;
        continue;
      }

      // Accumulate consecutive INC/DEC on the same register
      int Count = 0;
      bool CurrentIsInc = IsInc;
      auto ScanIt = MI;
      while (ScanIt != MBB.end()) {
        MachineInstr &S = *ScanIt;
        const char *SName = opcodeName(S.getOpcode());
        if (!SName)
          break;
        bool SIsInc = strstr(SName, "INC") != nullptr;
        bool SIsDec = strstr(SName, "DEC") != nullptr;
        if (!(SIsInc || SIsDec))
          break;

        // find reg for S
        unsigned SReg = 0;
        for (const MachineOperand &MO : S.operands()) {
          if (MO.isReg() && (MO.isDef() || MO.isUse())) {
            SReg = MO.getReg();
            break;
          }
        }
        if (SReg != Reg)
          break;

        // If INC/DEC alternate sign, we still accumulate algebraically:
        // INC INC DEC -> net +1
        if (SIsInc)
          Count += 1;
        else
          Count -= 1;

        ++ScanIt;
      }

      // If net count is zero, remove the sequence
      if (Count == 0) {
        // erase all instructions from MI up to ScanIt
        auto EraseIt = MI;
        while (EraseIt != ScanIt) {
          EraseIt = MBB.erase(EraseIt);
        }
        MI = ScanIt;
        Changed = true;
        continue;
      }

      // Create single ADD or SUB with immediate = abs(Count)
      bool NetIsAdd = Count > 0;
      unsigned Imm = (unsigned)std::abs(Count);

      unsigned NewOpc = selectAddSubOpcode(NetIsAdd, Width);
      DebugLoc DL = Instr.getDebugLoc();

      // Build new instruction: ADDri / SUBri
      // Typical operand pattern: dstReg, imm
      // For X86 ADDri opcodes the builder usage is:
      // BuildMI(MBB, MI, DL, TII->get(ADD32ri)).addReg(Reg,
      // RegState::Define).addImm(Imm); But exact operand order may vary; we try
      // a common pattern and preserve flags.
      MachineInstrBuilder MIB = BuildMI(MBB, MI, DL, TII->get(NewOpc));
      MIB.addReg(Reg, RegState::Define | RegState::Implicit);
      MIB.addImm(Imm);

      // Remove original sequence
      auto EraseIt = MI;
      while (EraseIt != ScanIt) {
        EraseIt = MBB.erase(EraseIt);
      }

      // Advance MI to the instruction after the inserted one
      // The inserted instruction is placed at position MI (where first INC
      // was). So we need to move MI to the next instruction after the inserted
      // one. Find the inserted instruction by looking backwards from ScanIt.
      MI = ScanIt;
      Changed = true;
    }
  }

  return Changed;
}

// Register the pass
static RegisterPass<ExamplePass>
    X("example-x86", "Replace INC/DEC with ADD/SUB", false, false);

} // namespace
