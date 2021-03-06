//===-- GBZ80InstrInfo.h - GBZ80 Instruction Information --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the GBZ80 implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef GBZ80INSTRINFO_H
#define GBZ80INSTRINFO_H

#include "GBZ80.h"
#include "GBZ80RegisterInfo.h"
#include "llvm/Target/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "GBZ80GenInstrInfo.inc"

namespace llvm {
  namespace GBZ80 {
    enum CondCode {
      COND_NZ = 0,
      COND_Z  = 1,
      COND_NC = 2,
      COND_C  = 3,
      COND_PO = 4,
      COND_PE = 5,
      COND_P  = 6,
      COND_M  = 7,

      COND_INVALID
    };
  } // end namespace GBZ80
  class GBZ80InstrInfo : public GBZ80GenInstrInfo {
    const GBZ80RegisterInfo RI;
    GBZ80TargetMachine &TM;
  public:
    explicit GBZ80InstrInfo(GBZ80TargetMachine &tm);

    virtual const GBZ80RegisterInfo &getRegisterInfo() const { return RI; }

    virtual void copyPhysReg(MachineBasicBlock &MBB,
      MachineBasicBlock::iterator I, DebugLoc DL,
      unsigned DestReg, unsigned SrcReg, bool KillSrc) const;

    virtual void storeRegToStackSlot(MachineBasicBlock &MBB,
      MachineBasicBlock::iterator MI, unsigned SrcReg, bool isKill,
      int FrameIndex, const TargetRegisterClass *RC,
      const TargetRegisterInfo *TRI) const;
    virtual void loadRegFromStackSlot(MachineBasicBlock &MBB,
      MachineBasicBlock::iterator MI, unsigned DestReg,
      int FrameIndex, const TargetRegisterClass *RC,
      const TargetRegisterInfo *TRI) const;

    virtual bool expandPostRAPseudo(MachineBasicBlock::iterator MI) const;

    virtual MachineInstr* commuteInstruction(MachineInstr *MI,
      bool NewMI = false) const;

    virtual bool AnalyzeBranch(MachineBasicBlock &MBB,
      MachineBasicBlock *&TBB, MachineBasicBlock *&FBB,
      SmallVectorImpl<MachineOperand> &Cond,
      bool AllowModify) const;
    virtual unsigned RemoveBranch(MachineBasicBlock &MBB) const;
    virtual unsigned InsertBranch(MachineBasicBlock &MBB,
      MachineBasicBlock *TBB, MachineBasicBlock *FBB,
      const SmallVectorImpl<MachineOperand> &Cond,
      DebugLoc DL) const;
    virtual bool ReverseBranchCondition(
      SmallVectorImpl<MachineOperand> &Cond) const;
  }; // end class Z80InstrInfo
} // end namespace llvm

#endif
