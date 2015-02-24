//===-- GBZ80RegisterInfo.td - GBZ80 Register defs ---------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the GBZ80 register file
//===----------------------------------------------------------------------===//


class GBZ80Register<string n, bits<16> Enc = 0> : Register<n> {
  let Namespace = "GBZ80";
  let HWEncoding = Enc;
}

class GBZ80SubRegIndex<int size, int offset = 0> : SubRegIndex<size, offset> {
  let Namespace = "GBZ80";
}

def subreg_hi : GBZ80SubRegIndex<8, 8>;
def subreg_lo : GBZ80SubRegIndex<8, 0>;

class GBZ80RegWithSubRegs<string n, bits<16> Enc = 0, list<Register> subregs = []>
  : GBZ80Register<n, Enc> {
  let SubRegs = subregs;
  let SubRegIndices = [subreg_hi, subreg_lo];
  let CoveredBySubRegs = 1;
}

class GBZ80Register8Class<dag reglist>
  : RegisterClass<"GBZ80", [i8], 8, reglist> {}

class GBZ80Register16Class<dag reglist>
  : RegisterClass<"GBZ80", [i16], 8, reglist> {}

// Accumulator
def A : GBZ80Register<"A", 7>;

// Auxiliary Registers
def B : GBZ80Register<"B", 0>;
def C : GBZ80Register<"C", 1>;
def D : GBZ80Register<"D", 2>;
def E : GBZ80Register<"E", 3>;
def H : GBZ80Register<"H", 4>;
def L : GBZ80Register<"L", 5>;

// Flags Register
def FLAGS : GBZ80Register<"F">;

// 16-bit registers
def AF : GBZ80RegWithSubRegs<"af", 0, [A, FLAGS]>;
def BC : GBZ80RegWithSubRegs<"bc", 1, [B, C]>;
def DE : GBZ80RegWithSubRegs<"de", 2, [D, E]>;
def HL : GBZ80RegWithSubRegs<"hl", 3, [H, L]>;

def SP : GBZ80Register<"SP">;
def PC : GBZ80Register<"PC">;

def GR8 : GBZ80Register8Class<(add A, B, C, D, E, H, L)>;
def GR16 : GBZ80Register16Class<(add BC, DE, HL)>;
