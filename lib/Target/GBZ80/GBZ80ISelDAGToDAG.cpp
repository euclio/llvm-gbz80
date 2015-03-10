//===- GBZ80ISelDAGToDAG.cpp - A DAG inst selector for GBZ80 --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines a DAG pattern matching instruction selector for GBZ80,
// converting from a legalized dag to a GBZ80 dag.
//
//===----------------------------------------------------------------------===//

#include "GBZ80.h"
#include "GBZ80TargetMachine.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

namespace {
  class GBZ80DAGToDAGISel : public SelectionDAGISel {
  public:
    GBZ80DAGToDAGISel(GBZ80TargetMachine &TM, CodeGenOpt::Level OptLevel)
      : SelectionDAGISel(TM, OptLevel)
    {}
#include "GBZ80GenDAGISel.inc"
  private:
    SDNode *Select(SDNode *N);
    bool SelectIAddr(SDValue N, SDValue &Addr);
  };
} // end namespace

// createGBZ80ISelDAG - This pass converts a legalized DAG into a
// GBZ80-specific DAG, ready for instruction scheduling.

FunctionPass *llvm::createGBZ80ISelDAG(GBZ80TargetMachine &TM,
  CodeGenOpt::Level OptLevel) {
  return new GBZ80DAGToDAGISel(TM, OptLevel);
}

SDNode *GBZ80DAGToDAGISel::Select(SDNode *Node)
{
  DebugLoc dl = Node->getDebugLoc();

  // // Dump information about the Node being selected
  // DEBUG(errs() << "Selecting: ";
  //   Node->dump(CurDAG);
  //   errs() << "\n");

  // // If we have a custom node, we already have selected
  // if (Node->isMachineOpcode()) {
  //   DEBUG(errs() << "== ";
  //     Node->dump(CurDAG);
  //     errs() << "\n");
  //   return NULL;
  // }

  switch (Node->getOpcode())
  {
  default: break;
  }

  // Select the default instruction
  SDNode *ResNode = SelectCode(Node);

  // if (ResNode == NULL || ResNode == Node)
  //   DEBUG(Node->dump(CurDAG));
  // else
  //   DEBUG(errs() << "\n");

  return ResNode;
}

bool GBZ80DAGToDAGISel::SelectIAddr(SDValue N, SDValue &Addr) {
    switch (N->getOpcode()) {
        case ISD::Constant:
            if (ConstantSDNode *CN = dyn_cast<ConstantSDNode>(N)) {
                Addr = CurDAG->getTargetConstant(CN->getZExtValue(), MVT::i16);
                return true;
            }
            break;
        case GBZ80ISD::WRAPPER:
            Addr = N->getOperand(0);
            return true;
    }
    return false;
}
