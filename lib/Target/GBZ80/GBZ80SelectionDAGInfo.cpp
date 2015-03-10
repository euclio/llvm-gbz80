//===-- GBZ80SelectionDAGInfo.cpp - GBZ80 SelectionDAG Info ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the GBZ80SelectionDAGInfo class.
//
//===----------------------------------------------------------------------===//

#include "GBZ80TargetMachine.h"
using namespace llvm;

GBZ80SelectionDAGInfo::GBZ80SelectionDAGInfo(const GBZ80TargetMachine &tm)
  : TargetSelectionDAGInfo(tm.getDataLayout())
{
}

GBZ80SelectionDAGInfo::~GBZ80SelectionDAGInfo()
{
}
