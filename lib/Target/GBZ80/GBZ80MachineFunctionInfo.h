//===-- GBZ80MachineFuctionInfo.h - GBZ80 machine function info -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares GBZ80-specific per-machine-function information.
//
//===----------------------------------------------------------------------===//

#ifndef GBZ80MACHINEFUNCTIONINFO_H
#define GBZ80MACHINEFUNCTIONINFO_H

#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {
  class GBZ80MachineFunctionInfo : public MachineFunctionInfo {
    virtual void anchor();

    // needFP - Flag which indicate to use FP
    bool needFP;

    // CalleeSavedFrameSize - Size of the callee-saved register portion of the
    // stack frame in bytes.
    unsigned CalleeSavedFrameSize;
  public:
    explicit GBZ80MachineFunctionInfo(MachineFunction &MF)
      : needFP(false), CalleeSavedFrameSize(0) {}

    unsigned getCalleeSavedFrameSize() { return CalleeSavedFrameSize; }
    void setCalleeSavedFrameSize(unsigned bytes) {
      CalleeSavedFrameSize = bytes;
    }
    bool isNeedFP() const { return needFP; }
    void setNeedFP() { needFP = true; }
  }; // end class GBZ80MachineFunctionInfo
} // end namespace llvm

#endif
