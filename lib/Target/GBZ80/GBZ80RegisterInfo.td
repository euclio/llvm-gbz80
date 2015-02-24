//===-- GBZ80RegisterInfo.td - GBZ80 Register defs -----------*- tablegen -*-===//
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


class GBZ80Reg<string n> : Register<n> {

}

def AF : Register<"AF">, DwarfRegNum<[0]>;
def BC : Register<"BC">, DwarfRegNum<[1]>;
def DE : Register<"DE">, DwarfRegNum<[2]>;
def HL : Register<"HL">, DwarfRegNum<[3]>;

def SP : Register<"SP">, DwarfRegNum<[4]>;
def PC : Register<"PC">, DwarfRegNum<[5]>;
