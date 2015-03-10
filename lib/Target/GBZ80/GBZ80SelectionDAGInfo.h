//===-- GBZ80SelectionDAGInfo.h - GBZ80 SelectionDAG Info -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the GBZ80 subclass for TargetSelectionDAGInfo.
//
//===----------------------------------------------------------------------===//

#ifndef GBZ80SELECTIONDAGINFO_H
#define GBZ80SELECTIONDAGINFO_H

#include "llvm/Target/TargetSelectionDAGInfo.h"

namespace llvm {
  class GBZ80SelectionDAGInfo : public TargetSelectionDAGInfo {
  public:
    explicit GBZ80SelectionDAGInfo(const GBZ80TargetMachine &tm);
    ~GBZ80SelectionDAGInfo();
  }; // end class GBZ80SelectionDAGInfo
} // end namespace llvm

#endif
