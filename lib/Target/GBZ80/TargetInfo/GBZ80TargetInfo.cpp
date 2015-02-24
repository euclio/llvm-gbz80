//===-- GBZ80TargetInfo.cpp - GBZ80 Target Implementation -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "GBZ80.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

Target llvm::TheGBZ80Target;

extern "C" void LLVMInitializeGBZ80TargetInfo() {
    RegisterTarget<Triple::gbz80, /*HasJIT=*/ false>
        X(TheGBZ80Target, "gbz80", "GBZ80");
}
