//===-- GBZ80TargetMachine.h - Define TargetMachine for GBZ80 ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the GBZ80 specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_GBZ80_GBZ80TARGETMACHINE_H
#define LLVM_LIB_TARGET_GBZ80_GBZ80TARGETMACHINE_H

#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

class GBZ80TargetMachine : public LLVMTargetMachine {
  const DataLayout DL;
public:
  GBZ80TargetMachine(const Target &T, StringRef TT, StringRef CPU,
                     StringRef FS, const TargetOptions &Options,
                     Reloc::Model RM, CodeModel::Model CM,
                     CodeGenOpt::Level OL);

  ~GBZ80TargetMachine() override;

  const DataLayout *getDataLayout() const override { return &DL; }

  // Pass Pipeline Configuration
  /*TargetPassConfig *createPassConfig(PassManagerBase &PM) override;
  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }*/
};

} // end namespace llvm

#endif
