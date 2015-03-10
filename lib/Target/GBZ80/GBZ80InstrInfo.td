//===-- GBZ80InstrInfo.td - GBZ80 Instruction defs ---------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source 
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file describes the GBZ80 instructions in TableGen format.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// GBZ80 Instruction Format Definitions.
//===----------------------------------------------------------------------===//

include "GBZ80InstrFormats.td"

//===----------------------------------------------------------------------===//
// GBZ80 Type Profiles.
//===----------------------------------------------------------------------===//
def SDT_GBZ80Call         : SDTypeProfile<0, -1, [SDTCisVT<0, iPTR>]>;
def SDT_GBZ80CallSeqStart : SDCallSeqStart<[SDTCisVT<0, i16>]>;
def SDT_GBZ80CallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, i16>, SDTCisVT<1, i16>]>;
def SDT_GBZ80Wrapper      : SDTypeProfile<1, 1, [SDTCisSameAs<0, 1>,
                                                 SDTCisPtrTy<0>]>;
def SDT_GBZ80Cp           : SDTypeProfile<0, 2, [SDTCisSameAs<0, 1>]>;
def SDT_GBZ80SelectCC     : SDTypeProfile<1, 3, [SDTCisSameAs<0, 1>,
                                                 SDTCisSameAs<1, 2>,
                                                 SDTCisVT<3, i8>]>;
def SDT_GBZ80BrCC         : SDTypeProfile<0, 2, [SDTCisVT<0, OtherVT>,
                                                 SDTCisVT<1, i8>]>;
def SDT_GBZ80Shift        : SDTypeProfile<1, 2, [SDTCisSameAs<0, 1>,
                                                 SDTCisVT<2, i8>]>;
//===----------------------------------------------------------------------===//
// GBZ80 Specific Node Definitions.
//===----------------------------------------------------------------------===//
def GBZ80call          : SDNode<"GBZ80ISD::CALL", SDT_GBZ80Call,
                       [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue, SDNPVariadic]>;
def GBZ80callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_GBZ80CallSeqStart,
                       [SDNPHasChain, SDNPOutGlue]>;
def GBZ80callseq_end   : SDNode<"ISD::CALLSEQ_END", SDT_GBZ80CallSeqEnd,
                       [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;
def GBZ80ret           : SDNode<"GBZ80ISD::RET", SDTNone,
                       [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
def GBZ80wrapper       : SDNode<"GBZ80ISD::WRAPPER", SDT_GBZ80Wrapper>;
def GBZ80rlc           : SDNode<"GBZ80ISD::RLC", SDTIntUnaryOp, [SDNPOutGlue]>;
def GBZ80rrc           : SDNode<"GBZ80ISD::RRC", SDTIntUnaryOp, [SDNPOutGlue]>;
def GBZ80rl            : SDNode<"GBZ80ISD::RL",  SDTIntUnaryOp,
                       [SDNPOutGlue, SDNPInGlue]>;
def GBZ80rr            : SDNode<"GBZ80ISD::RR",  SDTIntUnaryOp, 
                       [SDNPOutGlue, SDNPInGlue]>;
def GBZ80sla           : SDNode<"GBZ80ISD::SLA", SDTIntUnaryOp, [SDNPOutGlue]>;
def GBZ80sra           : SDNode<"GBZ80ISD::SRA", SDTIntUnaryOp, [SDNPOutGlue]>;
def GBZ80sll           : SDNode<"GBZ80ISD::SLL", SDTIntUnaryOp, [SDNPOutGlue]>;
def GBZ80srl           : SDNode<"GBZ80ISD::SRL", SDTIntUnaryOp, [SDNPOutGlue]>;
def GBZ80shl           : SDNode<"GBZ80ISD::SHL", SDT_GBZ80Shift, []>;
def GBZ80lshr          : SDNode<"GBZ80ISD::LSHR", SDT_GBZ80Shift, []>;
def GBZ80ashr          : SDNode<"GBZ80ISD::ASHR", SDT_GBZ80Shift, []>;
def GBZ80cp            : SDNode<"GBZ80ISD::CP", SDT_GBZ80Cp, [SDNPOutGlue]>;
def GBZ80selectcc      : SDNode<"GBZ80ISD::SELECT_CC", SDT_GBZ80SelectCC,
                       [SDNPInGlue]>;
def GBZ80brcc          : SDNode<"GBZ80ISD::BR_CC", SDT_GBZ80BrCC,
                       [SDNPHasChain, SDNPInGlue]>;
def GBZ80scf           : SDNode<"GBZ80ISD::SCF", SDTNone, [SDNPOutGlue]>;
def GBZ80ccf           : SDNode<"GBZ80ISD::CCF", SDTNone,
                       [SDNPOutGlue, SDNPInGlue]>;
//===----------------------------------------------------------------------===//
// Operand Definitions.
//===----------------------------------------------------------------------===//

def brtarget : Operand<OtherVT> {
  let EncoderMethod = "getBREncoding";
}

def calltarget : Operand<iPTR> {
  let EncoderMethod = "getBREncoding";
}

def cc : Operand<i8> {
  let PrintMethod = "printCCOperand";
}

//===----------------------------------------------------------------------===//
// Complex and other pattern definitions.
//===----------------------------------------------------------------------===//

def iaddr : ComplexPattern<iPTR, 1, "SelectIAddr", [], []>;

def rst : PatLeaf<(i16 imm:$dst), [{
  return (N->getZExtValue() & 0x38) == N->getZExtValue();
}]>;

//===----------------------------------------------------------------------===//
// Instruction list.
//===----------------------------------------------------------------------===//
let Defs = [SP], Uses = [SP] in {
  def ADJCALLSTACKDOWN : PseudoI<(outs), (ins i16imm:$amt),
    [(GBZ80callseq_start timm:$amt)]>;
  def ADJCALLSTACKUP   : PseudoI<(outs), (ins i16imm:$amt1, i16imm:$amt2),
    [(GBZ80callseq_end timm:$amt1, timm:$amt2)]>;
}

let usesCustomInserter = 1 in {
  def SELECT8  : PseudoI<(outs GR8:$dst),  (ins GR8:$src,  GR8:$src2,  cc:$cc),
    [(set GR8:$dst,  (GBZ80selectcc GR8:$src,  GR8:$src2,  imm:$cc))]>;
  def SELECT16 : PseudoI<(outs GR16:$dst), (ins GR16:$src, GR16:$src2, cc:$cc),
    [(set GR16:$dst, (GBZ80selectcc GR16:$src, GR16:$src2, imm:$cc))]>;
  def SHL8   : PseudoI<(outs GR8:$dst), (ins GR8:$src, GR8:$cnt),
    [(set GR8:$dst, (GBZ80shl GR8:$src, GR8:$cnt))]>;
  def LSHR8  : PseudoI<(outs GR8:$dst), (ins GR8:$src, GR8:$cnt),
    [(set GR8:$dst, (GBZ80lshr GR8:$src, GR8:$cnt))]>;
  def ASHR8  : PseudoI<(outs GR8:$dst), (ins GR8:$src, GR8:$cnt),
    [(set GR8:$dst, (GBZ80ashr GR8:$src, GR8:$cnt))]>;
  def SHL16  : PseudoI<(outs GR16:$dst), (ins GR16:$src, GR8:$cnt),
    [(set GR16:$dst, (GBZ80shl GR16:$src, GR8:$cnt))]>;
  def LSHR16 : PseudoI<(outs GR16:$dst), (ins GR16:$src, GR8:$cnt),
    [(set GR16:$dst, (GBZ80lshr GR16:$src, GR8:$cnt))]>;
  def ASHR16 : PseudoI<(outs GR16:$dst), (ins GR16:$src, GR8:$cnt),
    [(set GR16:$dst, (GBZ80ashr GR16:$src, GR8:$cnt))]>;
}
//===----------------------------------------------------------------------===//
//  Miscellaneous Instructions.
//===----------------------------------------------------------------------===//
let Defs = [FLAGS] in {
  def SCF : I<0x37, (outs), (ins), "scf", [(GBZ80scf)]>;
  let Uses = [FLAGS] in
  def CCF : I<0x3F, (outs), (ins), "ccf", [(GBZ80ccf)]>;
}

let Defs = [A, FLAGS], Uses = [A] in {
  def CPL : I<0x2F, (outs), (ins), "cpl", [(set A, (not A))]>;
  def NEG : I<0x44, (outs), (ins), "neg", [(set A, (ineg A))]>, ED;
}

let Uses = [SP] in 
  def ADD16rSP : I<0x39, (outs GR16:$dst), (ins), "add\t{$dst, sp}", []>;
let Defs = [SP] in
  def LD16SPr  : I<0xF9, (outs), (ins GR16:$src), "ld\t{sp, $src}", []>;
let Defs = [HL, DE], Uses = [HL, DE] in
  def EX_DE_HL : I<0xEB, (outs), (ins), "ex\t{de, hl}", []>;

let Defs = [SP], Uses = [SP] in {
  let mayLoad = 1 in
    def POP16r  : IRp<0xC1, (outs GR16:$reg), (ins), "pop\t{$reg}", []>;
  let mayStore = 1 in
    def PUSH16r : IRp<0xC5, (outs), (ins GR16:$reg), "push\t{$reg}", []>;
}
//===----------------------------------------------------------------------===//
// Control Flow Instructions.
//===----------------------------------------------------------------------===//
let isCall = 1, Uses = [SP] in {
  def CALL : II16<0xCD, (outs), (ins calltarget:$dst, variable_ops),
    "call\t{$dst}", [(GBZ80call imm:$dst)]>;
  def RST  : Irst<0xC7, (outs), (ins i16imm:$dst, variable_ops),
    "rst\t$dst", [(GBZ80call rst:$dst)]>;
}

let isReturn = 1, isTerminator = 1, isBarrier = 1 in
def RET : I<0xC9, (outs), (ins), "ret", [(GBZ80ret)]>;

let isBranch = 1, isTerminator = 1 in {
  let isBarrier = 1 in
  def JP : II16<0xC3, (outs), (ins brtarget:$dst),
    "jp\t{$dst}", [(br bb:$dst)]>;
  let Uses = [FLAGS] in
  def JPCC : IRyI16<0xC2, (outs), (ins brtarget:$dst, cc:$Ry),
    "jp\t{$Ry, $dst}", [(GBZ80brcc bb:$dst, imm:$Ry)]>;
}

//===----------------------------------------------------------------------===//
// Load Instructions.
//===----------------------------------------------------------------------===//
let hasSideEffects = 0 in
def LD8rr : IRyRz<0x40, (outs GR8:$dst), (ins GR8:$src),
  "ld\t{$dst, $src}", []>;

let isReMaterializable = 1, isAsCheapAsAMove = 1 in {
  def LD8ri  : IRyI8<0x06,  (outs GR8:$dst),  (ins i8imm:$src),
    "ld\t{$dst, $src}", [(set GR8:$dst, imm:$src)]>;
  def LD16ri : IRpI16<0x01, (outs GR16:$dst), (ins i16imm:$src),
    "ld\t{$dst, $src}", [(set GR16:$dst, imm:$src)]>;
}

let canFoldAsLoad = 1, isReMaterializable = 1 in {
  let Defs = [A] in
  def LD8Am : II16<0x3A, (outs), (ins i16imm:$src),
    "ld\t{a, ($src)}", [(set A, (load iaddr:$src))]>;
  let Uses = [HL] in
  def LD8rHL : IRy<0x46, (outs GR8:$dst), (ins),
    "ld\t{$dst, (hl)}", [(set GR8:$dst, (load HL))]>;
  def LD16rm : IRpI16<0x4B, (outs GR16:$Rp), (ins i16imm:$Imm),
    "ld\t{$Rp, ($Imm)}", [(set GR16:$Rp, (load iaddr:$Imm))]>, ED;
}
let Uses = [A] in
def LD8mA : II16<0x32, (outs), (ins i16imm:$dst),
  "ld\t{($dst), a}", [(store A, iaddr:$dst)]>;
let Uses = [HL] in {
  def LD8HLr : IRz<0x70, (outs), (ins GR8:$src),
    "ld\t{(hl), $src}", [(store GR8:$src, HL)]>;
  def LD8HLi : II8<0x36, (outs), (ins i8imm:$src),
    "ld\t{(hl), $src}", [(store (i8 imm:$src), HL)]>;
}
def LD16mr : IRpI16<0x43, (outs), (ins i16imm:$Imm, GR16:$Rp),
  "ld\t{($Imm), $Rp}", [(store GR16:$Rp, iaddr:$Imm)]>, ED;
//===----------------------------------------------------------------------===//
// Arithmetic Instructions
//===----------------------------------------------------------------------===//

class ALUOpType<bits<3> val, SDNode ops, string asm> {
  field bits<3> Value = val;
  SDNode Node = ops;
  string Asm = asm;
}

def ALU_ADD : ALUOpType<0, add,   "add\ta, ">;
def ALU_ADC : ALUOpType<1, adde,  "adc\ta, ">;
def ALU_SUB : ALUOpType<2, sub,   "sub\t">;
def ALU_SBC : ALUOpType<3, sube,  "sbc\ta, ">;
def ALU_AND : ALUOpType<4, and,   "and\t">;
def ALU_XOR : ALUOpType<5, xor,   "xor\t">;
def ALU_OR  : ALUOpType<6, or,    "or\t">;
def ALU_CP  : ALUOpType<7, GBZ80cp, "cp\t">;

class ALUIr<ALUOpType alu, list<dag> pattern>
  : IRz<0x80, (outs), (ins GR8:$Rz), alu.Asm#"$Rz", pattern> {
  ALUOpType aluType = alu;
  let Inst{5-3} = aluType.Value;
}
class ALUIi<ALUOpType alu, list<dag> pattern>
  : II8<0xC6, (outs), (ins i8imm:$Imm), alu.Asm#"$Imm", pattern> {
  ALUOpType aluType = alu;
  let Inst{5-3} = aluType.Value;
}

multiclass ALUI<ALUOpType alu> {
  def r  : ALUIr<alu, [(set A, (alu.Node A, GR8:$Rz))]>;
  def i  : ALUIi<alu, [(set A, (alu.Node A, imm:$Imm))]>;
}

multiclass ALUICP<ALUOpType alu> {
  def r  : ALUIr<alu, [(GBZ80cp A, GR8:$Rz)]>;
  def i  : ALUIi<alu, [(GBZ80cp A, imm:$Imm)]>;
}

class RSOpType<bits<3> val, SDNode ops, string asm> {
  field bits<3> Value = val;
  SDNode Node = ops;
  string Asm = asm;
}

def RS_RLC : RSOpType<0, GBZ80rlc, "rlc">;
def RS_RRC : RSOpType<1, GBZ80rrc, "rrc">;
def RS_RL  : RSOpType<2, GBZ80rl,  "rl">;
def RS_RR  : RSOpType<3, GBZ80rr,  "rr">;
def RS_SLA : RSOpType<4, GBZ80sla, "sla">;
def RS_SRA : RSOpType<5, GBZ80sra, "sra">;
def RS_SLL : RSOpType<6, GBZ80sll, "sll">;
def RS_SRL : RSOpType<7, GBZ80srl, "srl">;

class RSIr<RSOpType rs>
  : IRz<0x00, (outs GR8:$dst), (ins GR8:$src), rs.Asm#"\t$src",
  [(set GR8:$dst, (rs.Node GR8:$src))]>, CB {
  let Inst{5-3} = rs.Value;
  let Constraints = "$src = $dst";
}

multiclass RSI<RSOpType rs> {
  def r  : RSIr<rs>;
}

let Uses = [A], Defs = [A, FLAGS] in {
  let isCommutable = 1 in {
    defm ADD8 : ALUI<ALU_ADD>;
    let Uses = [A, FLAGS] in
    defm ADC8 : ALUI<ALU_ADC>;
    defm AND8 : ALUI<ALU_AND>;
    defm XOR8 : ALUI<ALU_XOR>;
    defm OR8  : ALUI<ALU_OR>;
  }
  defm SUB8 : ALUI<ALU_SUB>;
  let Uses = [A, FLAGS] in
  defm SBC8 : ALUI<ALU_SBC>;
  let Defs = [FLAGS] in
  defm CP8  : ALUICP<ALU_CP>;
}

let Defs = [FLAGS] in {
  defm RLC8 : RSI<RS_RLC>;
  defm RRC8 : RSI<RS_RRC>;
  let Uses = [FLAGS] in {
    defm RL8  : RSI<RS_RL>;
    defm RR8  : RSI<RS_RR>;
  }
  defm SLA8 : RSI<RS_SLA>;
  defm SRA8 : RSI<RS_SRA>;
  defm SLL8 : RSI<RS_SLL>;
  defm SRL8 : RSI<RS_SRL>;
}

let Uses = [HL], Defs = [HL, FLAGS] in {
  let isCommutable = 1 in {
    def ADD16r : IRp<0x09, (outs), (ins GR16:$src),
      "add\t{hl, $src}", [(set HL, (add HL, GR16:$src))]>;
    let Uses = [HL, FLAGS] in
    def ADC16r : IRp<0x4A, (outs), (ins GR16:$src),
      "adc\t{hl, $src}", [(set HL, (adde HL, GR16:$src))]>, ED;
  }
  let Uses = [HL, FLAGS] in
  def SBC16r : IRp<0x42, (outs), (ins GR16:$src),
    "sbc\t{hl, $src}", [(set HL, (sube HL, GR16:$src))]>, ED;
}

let Constraints = "$src = $dst" in {
  let Defs = [FLAGS] in {
    def INC8r : IRy<0x04, (outs GR8:$dst), (ins GR8:$src),
      "inc\t{$src}", [(set GR8:$dst, (add GR8:$src, 1))]>;
    def DEC8r : IRy<0x05, (outs GR8:$dst), (ins GR8:$src),
      "dec\t{$src}", [(set GR8:$dst, (add GR8:$src, -1))]>;
  }
  def INC16r : IRp<0x03, (outs GR16:$dst), (ins GR16:$src),
    "inc\t{$src}", [(set GR16:$dst, (add GR16:$src, 1))]>;
  def DEC16r : IRp<0x0B, (outs GR16:$dst), (ins GR16:$src),
    "dec\t{$src}", [(set GR16:$dst, (add GR16:$src, -1))]>;
}

//===----------------------------------------------------------------------===//
// Non-Instruction Patterns.
//===----------------------------------------------------------------------===//

// anyext
def : Pat<(i16 (anyext GR8:$src)),
  (SUBREG_TO_REG (i16 0), GR8:$src, subreg_lo)>;

// trunc
def : Pat<(i8 (trunc GR16:$src)), (EXTRACT_SUBREG GR16:$src, subreg_lo)>;

// add, sub
def : Pat<(addc A, GR8:$src), (ADD8r GR8:$src)>;
def : Pat<(addc A, imm:$src), (ADD8r imm:$src)>;

def : Pat<(addc HL, GR16:$src), (ADD16r GR16:$src)>;

def : Pat<(subc A, GR8:$src), (SUB8r GR8:$src)>;
def : Pat<(subc A, imm:$src), (SUB8r imm:$src)>;

// calls
def : Pat<(GBZ80call (i16 tglobaladdr:$dst)), (CALL tglobaladdr:$dst)>;
def : Pat<(GBZ80call (i16 texternalsym:$dst)), (CALL texternalsym:$dst)>;

// neg
def : Pat<(add (not A), 1), (NEG)>;

// GlobalAddress
def : Pat<(GBZ80wrapper tglobaladdr:$dst), (LD16ri tglobaladdr:$dst)>;
