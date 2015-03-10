//===-- GBZ80.h - Top-level interface for Cpu0 representation ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in
// the LLVM GBZ80 back-end.
//
//===----------------------------------------------------------------------===//

#ifndef TARGET_GBZ80_H
#define TARGET_GBZ80_H

#include "MCTargetDesc/GBZ80MCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
  class GBZ80TargetMachine;
  class FunctionPass;

  FunctionPass *createGBZ80ISelDAG(GBZ80TargetMachine &TM, CodeGenOpt::Level OptLevel);
} // end namespace llvm;

#endif
