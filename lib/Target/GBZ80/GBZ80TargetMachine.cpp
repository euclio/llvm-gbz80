//===-- GBZ80TargetMachine.cpp - Define TargetMachine for GBZ80 -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#include "GBZ80.h"
#include "GBZ80TargetMachine.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/PassManager.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

extern "C" void LLVMInitializeGBZ80Target() {
    // Register the target.
    RegisterTargetMachine<GBZ80TargetMachine> X(TheGBZ80Target);
}

/*
 * Describes the memory layout of the Nintendo Game Boy.
 * Notes:
 *  - Little endian
 *  - 2-byte pointers, preferred alignment: 8 (?)
 */
GBZ80TargetMachine::GBZ80TargetMachine(const Target &T, StringRef TT,
                                       StringRef CPU, StringRef FS,
                                       const TargetOptions &Options,
                                       Reloc::Model RM, CodeModel::Model CM,
                                       CodeGenOpt::Level OL)
  : LLVMTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL),
    DL("e-p:8:8") {

}

GBZ80TargetMachine::~GBZ80TargetMachine() {}
