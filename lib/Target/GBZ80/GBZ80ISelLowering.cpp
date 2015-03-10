//===-- GBZ80ISelLowering.cpp - GBZ80 DAG Lowering Implementation ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that GBZ80 uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#include "GBZ80ISelLowering.h"
#include "GBZ80.h"
#include "GBZ80TargetMachine.h"
#include "GBZ80MachineFunctionInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

GBZ80TargetLowering::GBZ80TargetLowering(GBZ80TargetMachine &TM)
  : TargetLowering(TM)
{
  addRegisterClass(MVT::i8, &GBZ80::GR8RegClass);
  addRegisterClass(MVT::i16, &GBZ80::GR16RegClass);

  computeRegisterProperties();

  setStackPointerRegisterToSaveRestore(GBZ80::SP);

  setBooleanContents(ZeroOrOneBooleanContent);

  setLoadExtAction(ISD::EXTLOAD, MVT::i8, MVT::i8, Expand);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::i8, MVT::i8, Expand);
  setLoadExtAction(ISD::SEXTLOAD, MVT::i8, MVT::i8, Expand);

  setTruncStoreAction(MVT::i16, MVT::i8, Expand);

  setOperationAction(ISD::LOAD,  MVT::i16, Custom);
  setOperationAction(ISD::STORE, MVT::i16, Custom);

  setOperationAction(ISD::ZERO_EXTEND, MVT::i16, Custom);
  setOperationAction(ISD::SIGN_EXTEND, MVT::i16, Custom);

  setOperationAction(ISD::SRL,  MVT::i8, Custom);
  setOperationAction(ISD::SHL,  MVT::i8, Custom);
  setOperationAction(ISD::SRA,  MVT::i8, Custom);
  setOperationAction(ISD::ROTL, MVT::i8, Custom);
  setOperationAction(ISD::ROTR, MVT::i8, Custom);
  setOperationAction(ISD::SRL,  MVT::i16, Custom);
  setOperationAction(ISD::SHL,  MVT::i16, Custom);
  setOperationAction(ISD::SRA,  MVT::i16, Custom);

  setOperationAction(ISD::SUB,  MVT::i16, Custom);
  setOperationAction(ISD::SUBC, MVT::i16, Custom);
  setOperationAction(ISD::AND,  MVT::i16, Custom);
  setOperationAction(ISD::OR,   MVT::i16, Custom);
  setOperationAction(ISD::XOR,  MVT::i16, Custom);

  setOperationAction(ISD::SELECT_CC, MVT::i8, Custom);
  setOperationAction(ISD::SELECT_CC, MVT::i16, Custom);
  setOperationAction(ISD::BR_CC, MVT::i8, Custom);
  setOperationAction(ISD::BR_CC, MVT::i16, Custom);

  setOperationAction(ISD::GlobalAddress, MVT::i16, Custom);
}

//===----------------------------------------------------------------------===//
//                      Calling Convention Implementation
//===----------------------------------------------------------------------===//

#include "GBZ80GenCallingConv.inc"

SDValue GBZ80TargetLowering::LowerFormalArguments(SDValue Chain,
  CallingConv::ID CallConv, bool isVarArg,
  const SmallVectorImpl<ISD::InputArg> &Ins,
  SDLoc dl, SelectionDAG &DAG,
  SmallVectorImpl<SDValue> &InVals) const
{
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo *MFI = MF.getFrameInfo();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  GBZ80MachineFunctionInfo *GBZ80FI = MF.getInfo<GBZ80MachineFunctionInfo>();

  // CCValAssign - represent the assignment of
  // the arguments to a location
  SmallVector<CCValAssign, 16> ArgLocs;

  // CCState - info about the registers and stack slot.
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), ArgLocs,
          *DAG.getContext());

  // Analyze Formal Arguments
  CCInfo.AnalyzeFormalArguments(Ins, CC_GBZ80);

  assert(!isVarArg && "Varargs not supported yet!");

  for (unsigned i = 0, e = ArgLocs.size(); i != e; i++)
  {
    SDValue ArgValue;
    unsigned VReg;

    CCValAssign &VA = ArgLocs[i];
    if (VA.isRegLoc())
    { // Argument passed in registers
      EVT RegVT = VA.getLocVT();
      switch (RegVT.getSimpleVT().SimpleTy)
      {
      default:
        {
#ifndef NDEBUG
          errs() << "LowerFormalArguments Unhandled argument type: "
            << RegVT.getSimpleVT().SimpleTy << "\n";
#endif
          llvm_unreachable(0);
        }
      case MVT::i8:
        VReg = MRI.createVirtualRegister(&GBZ80::GR8RegClass);
        MRI.addLiveIn(VA.getLocReg(), VReg);
        ArgValue = DAG.getCopyFromReg(Chain, dl, VReg, RegVT);
        InVals.push_back(ArgValue);
        break;
      case MVT::i16:
        VReg = MRI.createVirtualRegister(&GBZ80::GR16RegClass);
        MRI.addLiveIn(VA.getLocReg(), VReg);
        ArgValue = DAG.getCopyFromReg(Chain, dl, VReg, RegVT);
        InVals.push_back(ArgValue);
        break;
      }
    }
    else
    {
      assert(VA.isMemLoc());
      GBZ80FI->setNeedFP();

      SDValue InVal;

      // Load the argument to a virtual register
      unsigned Size = VA.getLocVT().getStoreSize();
      if (Size > 2)
        errs() << "LowerFormalArguments unhandled argument type: "
        << EVT(VA.getLocVT()).getEVTString() << "\n";

      // Create the frame index object for this incoming parameter...
      int FI = MFI->CreateFixedObject(Size, VA.getLocMemOffset(), true);

      // Create the SelectionDAG nodes corresponding to a load
      // from this parameter
      SDValue FIN = DAG.getFrameIndex(FI, MVT::i16);
      InVal = DAG.getLoad(VA.getLocVT(), dl, Chain, FIN,
        MachinePointerInfo::getFixedStack(FI),
        false, false, false, 0);

      InVals.push_back(InVal);
    }
  }
  return Chain;
}

SDValue GBZ80TargetLowering::LowerReturn(SDValue Chain,
  CallingConv::ID CallConv, bool isVarArg,
  const SmallVectorImpl<ISD::OutputArg> &Outs,
  const SmallVectorImpl<SDValue> &OutVals,
  SDLoc dl, SelectionDAG &DAG) const
{
  // CCValAssign - represent the assignment of
  // the return value to a location
  SmallVector<CCValAssign, 16> RVLocs;

  // CCState - Info about the registers and stack slot.
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs,
          *DAG.getContext());

  // Analyze return values.
  CCInfo.AnalyzeReturn(Outs, RetCC_GBZ80);

  SDValue Flag;
  SmallVector<SDValue, 4> RetOps(1, Chain);

  // Copy the result value into the output registers.
  for (unsigned i = 0; i != RVLocs.size(); i++)
  {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    Chain = DAG.getCopyToReg(Chain, dl, VA.getLocReg(), OutVals[i], Flag);

    // Guarantee the all emitted copies are stuck together,
    // avoiding something bad
    Flag = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  RetOps[0] = Chain;  // Update chain.

  if (Flag.getNode())
    RetOps.push_back(Flag);

  return DAG.getNode(GBZ80ISD::RET, dl, MVT::Other);
}

SDValue GBZ80TargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
  SmallVectorImpl<SDValue> &InVals) const
{
  SelectionDAG &DAG                     = CLI.DAG;
  SDLoc dl                              = CLI.DL;
  SmallVector<ISD::OutputArg, 32> &Outs = CLI.Outs;
  SmallVector<SDValue, 32> OutVals      = CLI.OutVals;
  SmallVector<ISD::InputArg, 32> &Ins   = CLI.Ins;
  SDValue Chain                         = CLI.Chain;
  SDValue Callee                        = CLI.Callee;
  bool &isTailCall                      = CLI.IsTailCall;
  CallingConv::ID CallConv              = CLI.CallConv;
  bool isVarArg                         = CLI.IsVarArg;

  MachineFunction &MF = DAG.getMachineFunction();
  // GBZ80 target does not yet support tail call optimization
  isTailCall = false;

  // Analyze operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
          ArgLocs, *DAG.getContext());

  CCInfo.AnalyzeCallOperands(Outs, CC_GBZ80);

  // Get a count of how many bytes are to be pushed on the stack
  unsigned NumBytes = CCInfo.getNextStackOffset();

  Chain = DAG.getCALLSEQ_START(Chain, DAG.getConstant(NumBytes,
    getPointerTy(), true), dl);

  SmallVector<std::pair<unsigned, SDValue>, 4> RegsToPass;
  SmallVector<SDValue, 12> MemOpChains;
  SDValue StackPtr;

  // Walk the register/memloc assignments, inserting copies/loads.
  for (unsigned i = 0, e = ArgLocs.size(); i != e; i++)
  {
    CCValAssign &VA = ArgLocs[i];

    SDValue Arg = OutVals[i];
    switch (VA.getLocInfo())
    {
    default: llvm_unreachable("Unknown loc info!");
    case CCValAssign::Full: break;
    case CCValAssign::SExt:
      Arg = DAG.getNode(ISD::SIGN_EXTEND, dl, VA.getLocVT(), Arg);
      break;
    case CCValAssign::ZExt:
      Arg = DAG.getNode(ISD::ZERO_EXTEND, dl, VA.getLocVT(), Arg);
      break;
    case CCValAssign::AExt:
      Arg = DAG.getNode(ISD::ANY_EXTEND, dl, VA.getLocVT(), Arg);
      break;
    }

    // Arguments that can be passed on register must be kept at RegsToPass
    // vector.
    if (VA.isRegLoc()) {
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
    }
    else
    {
      assert(VA.isMemLoc());

      unsigned FP = MF.getSubtarget().getRegisterInfo()->getFrameRegister(MF);
      if (StackPtr.getNode() == 0)
        StackPtr = DAG.getCopyFromReg(Chain, dl, FP, getPointerTy());

      SDValue PtrOff = DAG.getNode(ISD::ADD, dl, getPointerTy(),
        StackPtr, DAG.getIntPtrConstant(VA.getLocMemOffset()));

      SDValue MemOp;
      ISD::ArgFlagsTy Flags = Outs[i].Flags;

      if (Flags.isByVal()) assert(0 && "Not implemented yet!");

      MemOp = DAG.getStore(Chain, dl, Arg, PtrOff, MachinePointerInfo(),
        false, false, 0);

      MemOpChains.push_back(MemOp);
    }
  }

  // Transform all store nodes into one single node because all store nodes are
  // independent of each other.
  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other);

  // Build a sequence of copy-to-reg nodes chained together with token chain and
  // flag operands which copy the outgoing args into registers. The Flag is
  // necessary since all emitted instructions must be stuck together.
  SDValue Flag;

  for (unsigned i = 0, e = RegsToPass.size(); i != e; i++)
  {
    Chain = DAG.getCopyToReg(Chain, dl, RegsToPass[i].first,
      RegsToPass[i].second, Flag);
    Flag = Chain.getValue(1);
  }

  // If the callee is a GlobalAddress node (quite common, every direct call is)
  // turn it into a TargetGlobalAddress node so that legalize doesn't hack it.
  // Likewise ExternalSymbol -> TargetExternalSymbol.
  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee))
    Callee = DAG.getTargetGlobalAddress(G->getGlobal(), dl, MVT::i16);
  else if (ExternalSymbolSDNode *E = dyn_cast<ExternalSymbolSDNode>(Callee))
    Callee = DAG.getTargetExternalSymbol(E->getSymbol(), MVT::i16);

  // Returns a chain & a flag for retval copy to use.
  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  SmallVector<SDValue, 8> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);

  // Add a register mask operand representing the call-preserved registers.
  const TargetRegisterInfo *TRI = MF.getSubtarget().getRegisterInfo();
  const uint32_t *Mask = TRI->getCallPreservedMask(CallConv);
  Ops.push_back(DAG.getRegisterMask(Mask));

  // Add argument registers to the end of the list so that they are
  // known live into the call.
  for (unsigned i = 0, e = RegsToPass.size(); i != e; i++)
    Ops.push_back(DAG.getRegister(RegsToPass[i].first,
                                  RegsToPass[i].second.getValueType()));

  if (Flag.getNode())
    Ops.push_back(Flag);

  Chain = DAG.getNode(GBZ80ISD::CALL, dl, NodeTys);
  Flag = Chain.getValue(1);

  // Create the CALLSEQ_END node.
  Chain = DAG.getCALLSEQ_END(Chain,
    DAG.getConstant(NumBytes, getPointerTy(), true),
    DAG.getConstant(0, getPointerTy(), true),
    Flag, dl);

  Flag = Chain.getValue(1);

  // Handle result values, copying them out of physregs into vregs that we
  // return.
  return LowerCallResult(Chain, Flag, CallConv, isVarArg, Ins, dl, DAG, InVals);
}

SDValue GBZ80TargetLowering::LowerCallResult(SDValue Chain, SDValue Flag,
  CallingConv::ID CallConv, bool isVarArg,
  const SmallVectorImpl<ISD::InputArg> &Ins,
  SDLoc dl, SelectionDAG &DAG,
  SmallVectorImpl<SDValue> &InVals) const
{
  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
    RVLocs, *DAG.getContext());

  CCInfo.AnalyzeCallResult(Ins, RetCC_GBZ80);

  // Copy all of the result registers out of their specified physreg.
  for (unsigned i = 0, e = Ins.size(); i != e; i++) {
    Chain = DAG.getCopyFromReg(Chain, dl, RVLocs[i].getLocReg(),
      RVLocs[i].getValVT(), Flag).getValue(1);
    Flag = Chain.getValue(2);
    InVals.push_back(Chain.getValue(0));
  }
  return Chain;
}

//===----------------------------------------------------------------------===//
//                      Custom Lowering Implementation
//===----------------------------------------------------------------------===//

SDValue GBZ80TargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const
{
  switch (Op.getOpcode())
  {
  case ISD::ZERO_EXTEND:   return LowerZExt(Op, DAG);
  case ISD::SIGN_EXTEND:   return LowerSExt(Op, DAG);
  case ISD::SUB:
  case ISD::SUBC:          return LowerSUB(Op, DAG);
  case ISD::SRL:
  case ISD::SHL:
  case ISD::SRA:
  case ISD::ROTL:
  case ISD::ROTR:          return LowerShifts(Op, DAG);
  case ISD::AND:
  case ISD::OR:
  case ISD::XOR:           return LowerBinaryOp(Op, DAG);
  case ISD::SELECT_CC:     return LowerSelectCC(Op, DAG);
  case ISD::BR_CC:         return LowerBrCC(Op, DAG);
  case ISD::GlobalAddress: return LowerGlobalAddress(Op, DAG);
  case ISD::STORE:         return LowerStore(Op, DAG);
  case ISD::LOAD:          return LowerLoad(Op, DAG);
  default:
    llvm_unreachable("unimplemented operation");
  }
}

const char *GBZ80TargetLowering::getTargetNodeName(unsigned Opcode) const
{
  switch (Opcode)
  {
  default: return NULL;
  case GBZ80ISD::WRAPPER:   return "GBZ80ISD::WRAPPER";
  case GBZ80ISD::SCF:       return "GBZ80ISD::SCF";
  case GBZ80ISD::CCF:       return "GBZ80ISD::CCF";
  case GBZ80ISD::RLC:       return "GBZ80ISD::RLC";
  case GBZ80ISD::RRC:       return "GBZ80ISD::RRC";
  case GBZ80ISD::RL:        return "GBZ80ISD::RL";
  case GBZ80ISD::RR:        return "GBZ80ISD::RR";
  case GBZ80ISD::SLA:       return "GBZ80ISD::SLA";
  case GBZ80ISD::SRA:       return "GBZ80ISD::SRA";
  case GBZ80ISD::SLL:       return "GBZ80ISD::SLL";
  case GBZ80ISD::SRL:       return "GBZ80ISD::SRL";
  case GBZ80ISD::SHL:       return "GBZ80ISD::SHL";
  case GBZ80ISD::LSHR:      return "GBZ80ISD::LSHR";
  case GBZ80ISD::ASHR:      return "GBZ80ISD::ASHR";
  case GBZ80ISD::CP:        return "GBZ80ISD::CP";
  case GBZ80ISD::SELECT_CC: return "GBZ80ISD::SELECT_CC";
  case GBZ80ISD::BR_CC:     return "GBZ80ISD::BR_CC";
  case GBZ80ISD::CALL:      return "GBZ80ISD::CALL";
  case GBZ80ISD::RET:       return "GBZ80ISD::RET";
  }
}

SDValue GBZ80TargetLowering::LowerZExt(SDValue Op, SelectionDAG &DAG) const
{
  SDLoc dl(Op);
  SDValue Val = Op.getOperand(0);
  EVT VT      = Op.getValueType();

  assert(VT == MVT::i16 && "ZExt support only i16");

  SDValue Tmp = DAG.getConstant(0, VT.getHalfSizedIntegerVT(*DAG.getContext()));
  SDValue HI  = DAG.getTargetInsertSubreg(GBZ80::subreg_hi, dl, VT, DAG.getUNDEF(VT), Tmp);
  SDValue LO  = DAG.getTargetInsertSubreg(GBZ80::subreg_lo, dl, VT, HI, Val);
  return LO;
}

SDValue GBZ80TargetLowering::LowerSExt(SDValue Op, SelectionDAG &DAG) const
{
  SDLoc dl(Op);
  SDValue Val = Op.getOperand(0);
  EVT VT      = Op.getValueType();
  EVT HalfVT  = VT.getHalfSizedIntegerVT(*DAG.getContext());

  assert(VT == MVT::i16 && "SExt support only i16");

  // Generating the next code:
  // LD L,A
  // ADD A,L - set carry flag if negative (7 bit is set)
  // SBC A,A - turn to -1 if carry flag set
  // LD H,A
  SDValue LO   = DAG.getTargetInsertSubreg(GBZ80::subreg_lo, dl, VT, DAG.getUNDEF(VT), Val);
  SDValue Add  = DAG.getNode(ISD::ADDC, dl, DAG.getVTList(HalfVT, MVT::Glue), Val, Val);
  SDValue Flag = Add.getValue(1);
  SDValue Sub  = DAG.getNode(ISD::SUBE, dl, HalfVT, Add, Add, Flag);
  SDValue HI   = DAG.getTargetInsertSubreg(GBZ80::subreg_hi, dl, VT, LO, Sub);
  return HI;
}

SDValue GBZ80TargetLowering::LowerSUB(SDValue Op, SelectionDAG &DAG) const
{
  SDLoc dl(Op);
  SDValue Op0 = Op.getOperand(0);
  SDValue Op1 = Op.getOperand(1);
  EVT VT      = Op.getValueType();

  assert(VT == MVT::i16 && "Only i16 SUB can by lowered");

  // Generating next code:
  // SCF, CCF - clear carry flag (FIXME: must be replaced by AND A)
  // SBC HL, $Rp - sub without carry
  SDValue Flag;
  Flag = DAG.getNode(GBZ80ISD::SCF, dl, MVT::Glue);
  Flag = DAG.getNode(GBZ80ISD::CCF, dl, MVT::Glue, Flag);
  return DAG.getNode(ISD::SUBE, dl, DAG.getVTList(VT, MVT::Glue), Op0, Op1, Flag);
}

SDValue GBZ80TargetLowering::LowerShifts(SDValue Op, SelectionDAG &DAG) const
{
  SDLoc dl(Op);
  unsigned Opc = Op.getOpcode();
  SDNode *N    = Op.getNode();
  EVT VT       = Op.getValueType();

  if(!isa<ConstantSDNode>(N->getOperand(1)))
  {
    switch (N->getOpcode())
    {
    default: llvm_unreachable("Invalid shift opcode!");
    case ISD::SHL:
      Opc = GBZ80ISD::SHL;
      break;
    case ISD::SRL:
      Opc = GBZ80ISD::LSHR;
      break;
    case ISD::SRA:
      Opc = GBZ80ISD::ASHR;
      break;
    }
    return DAG.getNode(Opc, dl, VT, N->getOperand(0), N->getOperand(1));
  }

  uint64_t ShiftAmount = cast<ConstantSDNode>(N->getOperand(1))->getZExtValue();
  SDValue Victim = N->getOperand(0);

  switch (Opc)
  {
  default: llvm_unreachable("Invalid shift opcode");
  case ISD::SRL:
    Opc = GBZ80ISD::SRL;
    break;
  case ISD::SHL:
    Opc = GBZ80ISD::SLA;
    break;
  case ISD::SRA:
    Opc = GBZ80ISD::SRA;
    break;
  case ISD::ROTL:
    Opc = GBZ80ISD::RLC;
    break;
  case ISD::ROTR:
    Opc = GBZ80ISD::RRC;
    break;
  }

  if (VT == MVT::i16)
  {
    SDValue LO, HI, Flag;
    EVT HalfVT = VT.getHalfSizedIntegerVT(*DAG.getContext());
    SDVTList VTs = DAG.getVTList(HalfVT, MVT::Glue);
    LO = DAG.getTargetExtractSubreg(GBZ80::subreg_lo, dl, HalfVT, Victim);
    HI = DAG.getTargetExtractSubreg(GBZ80::subreg_hi, dl, HalfVT, Victim);
    while (ShiftAmount--) {
      if (Opc == GBZ80ISD::SLA) {
        LO = DAG.getNode(Opc, dl, VTs, LO);
        Flag = LO.getValue(1);
        HI = DAG.getNode(GBZ80ISD::RL, dl, HalfVT, HI, Flag);
      }
      else if (Opc == GBZ80ISD::SRL || Opc == GBZ80ISD::SRA) {
        HI = DAG.getNode(Opc, dl, VTs, HI);
        Flag = HI.getValue(1);
        LO = DAG.getNode(GBZ80ISD::RR, dl, HalfVT, LO, Flag);
      }
      else llvm_unreachable("Unsupported shift");
    }
    Victim = DAG.getTargetInsertSubreg(GBZ80::subreg_lo, dl, VT, DAG.getUNDEF(VT), LO);
    Victim = DAG.getTargetInsertSubreg(GBZ80::subreg_hi, dl, VT, Victim, HI);
  }
  else
    while (ShiftAmount--)
      Victim = DAG.getNode(Opc, dl, VT, Victim);

  return Victim;
}

SDValue GBZ80TargetLowering::LowerBinaryOp(SDValue Op, SelectionDAG &DAG) const
{
  SDLoc dl(Op);
  unsigned Opc = Op.getOpcode();
  SDValue LHS  = Op.getOperand(0);
  SDValue RHS  = Op.getOperand(1);
  EVT VT       = Op.getValueType();
  EVT HalfVT   = VT.getHalfSizedIntegerVT(*DAG.getContext());

  assert(VT == MVT::i16 && "Invalid type for lowering");

  SDValue LHS_LO, LHS_HI;
  SDValue RHS_LO, RHS_HI;
  SDValue LO, HI;
  SDValue Res;

  LHS_LO = DAG.getTargetExtractSubreg(GBZ80::subreg_lo, dl, HalfVT, LHS);
  LHS_HI = DAG.getTargetExtractSubreg(GBZ80::subreg_hi, dl, HalfVT, LHS);
  if (ConstantSDNode *CN = dyn_cast<ConstantSDNode>(RHS)) {
    RHS_LO = DAG.getConstant(CN->getZExtValue() & 0xFF, HalfVT);
    RHS_HI = DAG.getConstant(CN->getZExtValue()>>8 & 0xFF, HalfVT);
  } else {
    RHS_LO = DAG.getTargetExtractSubreg(GBZ80::subreg_lo, dl, HalfVT, RHS);
    RHS_HI = DAG.getTargetExtractSubreg(GBZ80::subreg_hi, dl, HalfVT, RHS);
  }

  LO = DAG.getNode(Opc, dl, HalfVT, LHS_LO, RHS_LO);
  HI = DAG.getNode(Opc, dl, HalfVT, LHS_HI, RHS_HI);

  Res = DAG.getTargetInsertSubreg(GBZ80::subreg_lo, dl, VT, DAG.getUNDEF(VT), LO);
  Res = DAG.getTargetInsertSubreg(GBZ80::subreg_hi, dl, VT, Res, HI);

  return Res;
}

SDValue GBZ80TargetLowering::EmitCMP(SDValue &LHS, SDValue &RHS,
        SDValue &GBZ80CC, ISD::CondCode CC, SDLoc dl, SelectionDAG &DAG) const {
  EVT VT = LHS.getValueType();

  assert(!VT.isFloatingPoint() && "We don't handle FP yet");
  assert((VT == MVT::i8 || VT == MVT::i16) && "Invalid type in EmitCMP");

  GBZ80::CondCode TCC = GBZ80::COND_INVALID;
  switch (CC)
  {
  case ISD::SETUNE:
  case ISD::SETNE:
    TCC = GBZ80::COND_NZ;
    break;
  case ISD::SETUEQ:
  case ISD::SETEQ:
    TCC = GBZ80::COND_Z;
    break;
  case ISD::SETUGT:
    std::swap(LHS, RHS);
  case ISD::SETULT:
    TCC = GBZ80::COND_C;
    break;
  case ISD::SETULE:
    std::swap(LHS, RHS);
  case ISD::SETUGE:
    TCC = GBZ80::COND_NC;
    break;
  case ISD::SETLE:
    std::swap(LHS, RHS);
  case ISD::SETGE:
    TCC = GBZ80::COND_P;
    break;
  case ISD::SETGT:
    std::swap(LHS, RHS);
  case ISD::SETLT:
    TCC = GBZ80::COND_M;
    break;
  default: llvm_unreachable("Invalid integer condition!");
  }
  GBZ80CC = DAG.getConstant(TCC, MVT::i8);

  if (VT == MVT::i8)
    return DAG.getNode(GBZ80ISD::CP, dl, MVT::Glue, LHS, RHS);
  else // MVT::i16
    return DAG.getNode(ISD::SUBC, dl, DAG.getVTList(VT, MVT::Glue), LHS, RHS).getValue(1);
}

SDValue GBZ80TargetLowering::LowerSelectCC(SDValue Op, SelectionDAG &DAG) const
{
  SDLoc dl(Op);
  EVT VT           = Op.getValueType();
  SDValue LHS      = Op.getOperand(0);
  SDValue RHS      = Op.getOperand(1);
  SDValue TrueV    = Op.getOperand(2);
  SDValue FalseV   = Op.getOperand(3);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(4))->get();

  SDValue GBZ80CC;
  SDValue Flag = EmitCMP(LHS, RHS, GBZ80CC, CC, dl, DAG);

  return DAG.getNode(GBZ80ISD::SELECT_CC, dl, VT, TrueV, FalseV, GBZ80CC, Flag);
}

SDValue GBZ80TargetLowering::LowerBrCC(SDValue Op, SelectionDAG &DAG) const
{
  SDLoc dl(Op);
  EVT VT           = Op.getValueType();
  SDValue Chain    = Op.getOperand(0);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(1))->get();
  SDValue LHS      = Op.getOperand(2);
  SDValue RHS      = Op.getOperand(3);
  SDValue Dest     = Op.getOperand(4);

  SDValue GBZ80CC;
  SDValue Flag = EmitCMP(LHS, RHS, GBZ80CC, CC, dl, DAG);

  return DAG.getNode(GBZ80ISD::BR_CC, dl, VT, Chain, Dest, GBZ80CC, Flag);
}

SDValue GBZ80TargetLowering::LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const
{
  SDLoc dl(Op);
  EVT VT      = getPointerTy();

  const GlobalValue *GV = cast<GlobalAddressSDNode>(Op)->getGlobal();
  int64_t Offset = cast<GlobalAddressSDNode>(Op)->getOffset();

  // Create the TargetGlobalAddress node, folding in the constant offset.
  SDValue Result = DAG.getTargetGlobalAddress(GV, dl, VT, Offset);

  return DAG.getNode(GBZ80ISD::WRAPPER, dl, VT, Result);
}

SDValue GBZ80TargetLowering::LowerStore(SDValue Op, SelectionDAG &DAG) const
{
  SDLoc dl(Op);
  StoreSDNode *ST = dyn_cast<StoreSDNode>(Op);
  SDValue Chain   = ST->getChain();
  SDValue BasePtr = ST->getBasePtr();
  SDValue Value   = ST->getOperand(1);

  assert(!ST->isTruncatingStore() && "Truncating Store isn't supported yet!");
  assert(!ST->isIndexed() && "Indexed Store isn't supported yet!");
  
  switch (BasePtr.getOpcode())
  {
  default: break;
  case ISD::CopyFromReg:
      if (RegisterSDNode *RN = dyn_cast<RegisterSDNode>(BasePtr.getOperand(1)))
        if (RN->getReg() != getTargetMachine().getSubtargetImpl()->getRegisterInfo()->getFrameRegister(
          DAG.getMachineFunction()))
          break;
  case ISD::FrameIndex:
    return SDValue();
  }

  SDValue Lo, Hi;
  if (ConstantSDNode *CN = dyn_cast<ConstantSDNode>(Value)) {
    Lo = DAG.getConstant(CN->getZExtValue() & 0xFF, MVT::i8);
    Hi = DAG.getConstant((CN->getZExtValue()>>8) & 0xFF, MVT::i8);
  }
  else {
    Lo = DAG.getTargetExtractSubreg(GBZ80::subreg_lo, dl, MVT::i8, Value);
    Hi = DAG.getTargetExtractSubreg(GBZ80::subreg_hi, dl, MVT::i8, Value);
  }
  SDValue StoreLow = DAG.getStore(Chain, dl, Lo, BasePtr,
    ST->getPointerInfo(), ST->isVolatile(),
    ST->isNonTemporal(), ST->getAlignment());
  SDValue HighAddr = DAG.getNode(ISD::ADD, dl, MVT::i16, BasePtr,
    DAG.getConstant(1, MVT::i16));
  SDValue StoreHigh = DAG.getStore(Chain, dl, Hi, HighAddr,
    ST->getPointerInfo(), ST->isVolatile(),
    ST->isNonTemporal(), ST->getAlignment());
  return DAG.getNode(ISD::TokenFactor, dl, MVT::Other, StoreLow, StoreHigh);
}

SDValue GBZ80TargetLowering::LowerLoad(SDValue Op, SelectionDAG &DAG) const
{
  SDLoc dl(Op);
  LoadSDNode *LD  = cast<LoadSDNode>(Op);
  SDValue Chain   = LD->getChain();
  SDValue BasePtr = LD->getBasePtr();

  assert(!LD->isIndexed() && "Indexed load isn't supported yet!");
  assert(LD->getExtensionType() == ISD::NON_EXTLOAD && "Extload isn't supported yet!");

  switch (BasePtr.getOpcode())
  {
  default: break;
  case ISD::FrameIndex:
    return SDValue();
  }
  SDValue Lo = DAG.getLoad(MVT::i8, dl, Chain, BasePtr,
    MachinePointerInfo(), LD->isVolatile(), LD->isNonTemporal(),
    LD->isInvariant(), LD->getAlignment());
  SDValue HighAddr = DAG.getNode(ISD::ADD, dl, MVT::i16, BasePtr,
    DAG.getConstant(1, MVT::i16));
  SDValue Hi = DAG.getLoad(MVT::i8, dl, Chain, HighAddr,
    MachinePointerInfo(), LD->isVolatile(), LD->isNonTemporal(),
    LD->isInvariant(), LD->getAlignment());
  SDValue NewChain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other,
    Lo.getValue(1), Hi.getValue(1));

  SDValue LoRes = DAG.getTargetInsertSubreg(GBZ80::subreg_lo, dl,
    MVT::i16, DAG.getUNDEF(MVT::i16), Lo);
  SDValue HiRes = DAG.getTargetInsertSubreg(GBZ80::subreg_hi, dl,
    MVT::i16, LoRes, Hi);

  SDValue Ops[] = { HiRes, NewChain };
  return DAG.getMergeValues(Ops, dl);
}

//===----------------------------------------------------------------------===//
//                   Instructions With Custom Inserter
//===----------------------------------------------------------------------===//

MachineBasicBlock* GBZ80TargetLowering::EmitInstrWithCustomInserter(MachineInstr *MI,
  MachineBasicBlock *MBB) const
{
  unsigned Opc = MI->getOpcode();

  switch (Opc)
  {
  case GBZ80::SELECT8:
  case GBZ80::SELECT16: return EmitSelectInstr(MI, MBB);
  case GBZ80::SHL8:
  case GBZ80::LSHR8:
  case GBZ80::ASHR8:
  case GBZ80::SHL16:
  case GBZ80::LSHR16:
  case GBZ80::ASHR16:   return EmitShiftInstr(MI, MBB);
  default: llvm_unreachable("Invalid Custom Inserter Instruction");
  }
}

MachineBasicBlock* GBZ80TargetLowering::EmitSelectInstr(MachineInstr *MI,
  MachineBasicBlock *MBB) const
{
  DebugLoc dl = MI->getDebugLoc();
  const TargetInstrInfo &TII = *getTargetMachine().getSubtargetImpl()->getInstrInfo();

  const BasicBlock *LLVM_BB = MBB->getBasicBlock();
  MachineFunction::iterator I = MBB;
  I++;

  MachineBasicBlock *thisMBB = MBB;
  MachineFunction *MF = MBB->getParent();
  MachineBasicBlock *copy0MBB = MF->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *copy1MBB = MF->CreateMachineBasicBlock(LLVM_BB);
  MF->insert(I, copy0MBB);
  MF->insert(I, copy1MBB);

  copy1MBB->splice(copy1MBB->begin(), MBB,
    std::next(MachineBasicBlock::iterator(MI)), MBB->end());
  copy1MBB->transferSuccessorsAndUpdatePHIs(MBB);
  MBB->addSuccessor(copy0MBB);
  MBB->addSuccessor(copy1MBB);

  BuildMI(MBB, dl, TII.get(GBZ80::JPCC))
    .addMBB(copy1MBB)
    .addImm(MI->getOperand(3).getImm());

  MBB = copy0MBB;
  MBB->addSuccessor(copy1MBB);

  MBB = copy1MBB;
  BuildMI(*MBB, MBB->begin(), dl, TII.get(GBZ80::PHI),
    MI->getOperand(0).getReg())
    .addReg(MI->getOperand(1).getReg()).addMBB(thisMBB)
    .addReg(MI->getOperand(2).getReg()).addMBB(copy0MBB);

  MI->eraseFromParent();
  return MBB;
}

MachineBasicBlock* GBZ80TargetLowering::EmitShiftInstr(MachineInstr *MI,
  MachineBasicBlock *MBB) const
{
  MachineFunction *MF = MBB->getParent();
  MachineRegisterInfo &MRI = MF->getRegInfo();
  DebugLoc dl = MI->getDebugLoc();
  const TargetInstrInfo &TII = *getTargetMachine().getSubtargetImpl()->getInstrInfo();

  unsigned Opc, Opc2 = 0;
  const TargetRegisterClass *RC;

  switch (MI->getOpcode())
  {
  default: llvm_unreachable("Invalid shift opcode!");
  case GBZ80::SHL8:
    Opc = GBZ80::SLA8r;
    RC = &GBZ80::GR8RegClass;
    break;
  case GBZ80::LSHR8:
    Opc = GBZ80::SRL8r;
    RC = &GBZ80::GR8RegClass;
    break;
  case GBZ80::ASHR8:
    Opc = GBZ80::SRA8r;
    RC = &GBZ80::GR8RegClass;
    break;
  case GBZ80::SHL16:
    Opc = GBZ80::SLA8r;
    Opc2 = GBZ80::RL8r;
    RC = &GBZ80::GR16RegClass;
    break;
  case GBZ80::LSHR16:
    Opc = GBZ80::SRL8r;
    Opc2 = GBZ80::RR8r;
    RC = &GBZ80::GR16RegClass;
    break;
  case GBZ80::ASHR16:
    Opc = GBZ80::SRA8r;
    Opc2 = GBZ80::RR8r;
    RC = &GBZ80::GR16RegClass;
    break;
  }

  const BasicBlock *LLVM_BB = MBB->getBasicBlock();
  MachineFunction::iterator I = MBB;
  I++;

  MachineBasicBlock *LoopMBB = MF->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *RemMBB  = MF->CreateMachineBasicBlock(LLVM_BB);
  MF->insert(I, LoopMBB);
  MF->insert(I, RemMBB);

  RemMBB->splice(RemMBB->begin(), MBB,
    std::next<MachineBasicBlock::iterator>(MI), MBB->end());
  RemMBB->transferSuccessorsAndUpdatePHIs(MBB);

  // Add edges MBB => LoopMBB => RemMBB, MBB => RemMBB, LoopMBB => LoopMBB
  MBB->addSuccessor(LoopMBB);
  MBB->addSuccessor(RemMBB);
  LoopMBB->addSuccessor(RemMBB);
  LoopMBB->addSuccessor(LoopMBB);

  unsigned ShiftReg  = MRI.createVirtualRegister(RC);
  unsigned ShiftReg2 = MRI.createVirtualRegister(RC);
  unsigned ShiftAmt  = MRI.createVirtualRegister(&GBZ80::GR8RegClass);
  unsigned ShiftAmt2 = MRI.createVirtualRegister(&GBZ80::GR8RegClass);
  unsigned DstReg = MI->getOperand(0).getReg();
  unsigned SrcReg = MI->getOperand(1).getReg();
  unsigned AmtReg = MI->getOperand(2).getReg();

  // MBB:
  // LD A,AmtReg
  // CP 0
  // JP Z,RemMBB
  BuildMI(MBB, dl, TII.get(GBZ80::COPY), GBZ80::A).addReg(AmtReg);
  BuildMI(MBB, dl, TII.get(GBZ80::CP8i)).addImm(0);
  BuildMI(MBB, dl, TII.get(GBZ80::JPCC)).addMBB(RemMBB).addImm(GBZ80::COND_Z);

  // LoopMBB:
  // ShiftReg = phi [ %SrcReg, MBB ], [ %ShiftReg2, LoopMBB ]
  // ShiftAmt = phi [ %AmtReg, MBB ], [ %ShiftAmt2, LoopMBB ]
  // ShiftReg2 = Opc ShiftReg
  // ShiftAmt2 = DEC ShiftAmt
  // JP NZ, LoopMBB
  BuildMI(LoopMBB, dl, TII.get(GBZ80::PHI), ShiftReg)
    .addReg(SrcReg).addMBB(MBB)
    .addReg(ShiftReg2).addMBB(LoopMBB);
  BuildMI(LoopMBB, dl, TII.get(GBZ80::PHI), ShiftAmt)
    .addReg(AmtReg).addMBB(MBB)
    .addReg(ShiftAmt2).addMBB(LoopMBB);
  if (Opc2)
  {
    unsigned RegLo = MRI.createVirtualRegister(&GBZ80::GR8RegClass);
    unsigned RegHi = MRI.createVirtualRegister(&GBZ80::GR8RegClass);
    unsigned RegLo2 = MRI.createVirtualRegister(&GBZ80::GR8RegClass);
    unsigned RegHi2 = MRI.createVirtualRegister(&GBZ80::GR8RegClass);
    unsigned SupReg = MRI.createVirtualRegister(&GBZ80::GR16RegClass);
    unsigned Idx1 = GBZ80::subreg_lo;
    unsigned Idx2 = GBZ80::subreg_hi;

    // If the shift to the right, then swap subindices
    if (Opc2 == GBZ80::RR8r)
      std::swap(Idx1, Idx2);

    // Load subindices into GR8 registers.
    BuildMI(LoopMBB, dl, TII.get(GBZ80::COPY), RegLo)
      .addReg(ShiftReg, 0, Idx1);
    BuildMI(LoopMBB, dl, TII.get(GBZ80::COPY), RegHi)
      .addReg(ShiftReg, 0, Idx2);

    // Shift registers.
    BuildMI(LoopMBB, dl, TII.get(Opc), RegLo2).addReg(RegLo);
    BuildMI(LoopMBB, dl, TII.get(Opc2), RegHi2).addReg(RegHi);

    // Insert registers into super register.
    BuildMI(LoopMBB, dl, TII.get(GBZ80::INSERT_SUBREG), SupReg)
      .addReg(ShiftReg)
      .addReg(RegLo2)
      .addImm(Idx1);
    BuildMI(LoopMBB, dl, TII.get(GBZ80::INSERT_SUBREG), ShiftReg2)
      .addReg(SupReg)
      .addReg(RegHi2)
      .addImm(Idx2);
  }
  else // Shift 8 bit register
    BuildMI(LoopMBB, dl, TII.get(Opc), ShiftReg2)
      .addReg(ShiftReg);
  BuildMI(LoopMBB, dl, TII.get(GBZ80::DEC8r), ShiftAmt2)
    .addReg(ShiftAmt);
  BuildMI(LoopMBB, dl, TII.get(GBZ80::JPCC))
    .addMBB(LoopMBB).addImm(GBZ80::COND_NZ);

  // RemMBB:
  // DstReg = phi [ %SrcReg, MBB ], [ %ShiftReg2, LoopMBB ]
  BuildMI(*RemMBB, RemMBB->begin(), dl, TII.get(GBZ80::PHI), DstReg)
    .addReg(SrcReg).addMBB(MBB)
    .addReg(ShiftReg2).addMBB(LoopMBB);

  MI->eraseFromParent();
  return RemMBB;
}
