//===-- GBZ80RegisterInfo.cpp - GBZ80 Register Information ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the GBZ80 implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "GBZ80RegisterInfo.h"
#include "GBZ80.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetInstrInfo.h"

#define GET_REGINFO_TARGET_DESC
#include "GBZ80GenRegisterInfo.inc"

using namespace llvm;

GBZ80RegisterInfo::GBZ80RegisterInfo(GBZ80TargetMachine &tm, const TargetInstrInfo &tii)
  : Gbz80GenRegisterInfo(GBZ80::PC), TM(tm), TII(tii) {}

const uint16_t* GBZ80RegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  static const uint16_t CalleeSavedRegs[] = {
    GBZ80::BC, GBZ80::DE, 0
  };
  return CalleeSavedRegs;
}

const uint32_t* GBZ80RegisterInfo::getCallPreservedMask(CallingConv::ID CallConv) const
{
  return CSR_16_RegMask;
}

BitVector GBZ80RegisterInfo::getReservedRegs(const MachineFunction &MF) const
{
  BitVector Reserved(getNumRegs());

  Reserved.set(GBZ80::PC);
  Reserved.set(GBZ80::SP);
  Reserved.set(GBZ80::FLAGS);
  Reserved.set(getFrameRegister(MF));

  return Reserved;
}
