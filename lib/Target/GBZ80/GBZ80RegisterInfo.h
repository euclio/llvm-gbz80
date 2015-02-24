//===-- GBZ80RegisterInfo.h - Gbz80 Register Information Impl ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Gbz80 implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_GBZ80_GBZ80REGISTERINFO_H
#define LLVM_LIB_TARGET_GBZ80_GBZ80REGISTERINFO_H

#include "llvm/Target/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "GBZ80GenRegisterInfo.inc"

namespace llvm {
  class TargetInstrInfo;
  class GBZ80TargetMachine;

  class GBZ80RegisterInfo : public Gbz80GenRegisterInfo {
    GBZ80TargetMachine &TM;
    const TargetInstrInfo &TII;
  public:
    GBZ80RegisterInfo(GBZ80TargetMachine &tm, const TargetInstrInfo &tii);

    // Code Generation virtual methods...
    const uint16_t *getCalleeSavedRegs(const MachineFunction *MF = 0) const;
    const uint32_t *getCallPreservedMask(CallingConv::ID CallConv) const;

    BitVector getReservedRegs(const MachineFunction &MF) const;
  };
} // end namespace llvm

#endif
