//===-- GBZ80TargetFrameLowering.h - Define frame lowering for GBZ80 C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This class implements GBZ80-specific bits of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#ifndef GBZ80FRAMELOWERING_H
#define GBZ80FRAMELOWERING_H

#include "llvm/Target/TargetFrameLowering.h"

namespace llvm {
  class GBZ80TargetMachine;

  class GBZ80FrameLowering : public TargetFrameLowering {
    const GBZ80TargetMachine &TM;
  public:
    explicit GBZ80FrameLowering(const GBZ80TargetMachine &tm);

    void emitPrologue(MachineFunction &MF) const;
    void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const;

    void processFunctionBeforeCalleeSavedScan(MachineFunction &MF,
      RegScavenger *RS = NULL) const;

    void eliminateCallFramePseudoInstr(MachineFunction &MF,
      MachineBasicBlock &MBB, MachineBasicBlock::iterator I) const;

    bool spillCalleeSavedRegisters(MachineBasicBlock &MBB,
      MachineBasicBlock::iterator MI,
      const std::vector<CalleeSavedInfo> &CSI,
      const TargetRegisterInfo *TRI) const;
    bool restoreCalleeSavedRegisters(MachineBasicBlock &MBB,
      MachineBasicBlock::iterator MI,
      const std::vector<CalleeSavedInfo> &CSI,
      const TargetRegisterInfo *TRI) const;

    bool hasFP(const MachineFunction &MF) const;
  }; // end class GBZ80FrameLowering
} // end namespace llvm

#endif
