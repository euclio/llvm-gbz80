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

#include "GBZ80.h"
#include "GBZ80FrameLowering.h"
#include "GBZ80ISelLowering.h"
#include "GBZ80InstrInfo.h"
#include "GBZ80SelectionDAGInfo.h"
#include "llvm/IR/DataLayout.h"

namespace llvm {

class GBZ80TargetMachine : public LLVMTargetMachine {
  const DataLayout DL;
  GBZ80FrameLowering FrameLowering;
  GBZ80InstrInfo InstrInfo;
  GBZ80SelectionDAGInfo TSInfo;
  GBZ80TargetLowering TLInfo;
public:
  GBZ80TargetMachine(const Target &T, StringRef TT, StringRef CPU,
                     StringRef FS, const TargetOptions &Options,
                     Reloc::Model RM, CodeModel::Model CM,
                     CodeGenOpt::Level OL);

  const DataLayout *getDataLayout() const override { return &DL; }
  virtual const GBZ80FrameLowering *getFrameLowering() const {
      return &FrameLowering;
  }
  virtual const GBZ80InstrInfo *getInstrInfo() const { return &InstrInfo; }
  virtual const GBZ80RegisterInfo *getRegisterInfo() const {
      return &getInstrInfo()->getRegisterInfo();
  }
  virtual const GBZ80SelectionDAGInfo *getSelectionDAGInfo() const {
      return &TSInfo;
  }
  virtual const GBZ80TargetLowering *getTargetLowering() const {
      return &TLInfo;
  }

  // Pass Pipeline Configuration
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;
};

} // end namespace llvm

#endif
