//===-- VideoCore4RegisterInfo.td - VideoCore4 Register defs --*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source 
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the VideoCore4 register file
//===----------------------------------------------------------------------===//

class VideoCore4Reg<bits<4> num, string n> : Register<n> {
  field bits<4> Num = num;
  let Namespace = "VideoCore4";
}

//===----------------------------------------------------------------------===//
//  Registers
//===----------------------------------------------------------------------===//

foreach i = 0-23 in {
	def R#i : VideoCore4Reg<0, "r"#i>;
}
def GP  : VideoCore4Reg<0, "gp">;
def SP  : VideoCore4Reg<0, "sp">;
def LR  : VideoCore4Reg<0, "lr">;
def R27 : VideoCore4Reg<0, "r27">;
def ESP : VideoCore4Reg<0, "esp">;
def R29 : VideoCore4Reg<0, "r29">;
def SR  : VideoCore4Reg<0, "sr">;
def PC  : VideoCore4Reg<0, "pc">;

//===----------------------------------------------------------------------===//
//  Register Classes
//===----------------------------------------------------------------------===//

/* All registers */
def GR32 : RegisterClass<"VideoCore4", [i32, f32], 32,
    (add (sequence "R%u", 0, 23), GP, SP, LR, R27, R29)>;

/* Fast registers */
def FR32 : RegisterClass<"VideoCore4", [i32, f32], 32,
    (sequence "R%u", 0, 15)>;

/* Indexable registers */
def IR32 : RegisterClass<"VideoCore4", [i32], 32,
	(add R0, GP, SP)>;

def SR_CLASS : RegisterClass<"VideoCore4", [i32], 32,
	(add SR)>;

def PUSHPOP_START_REGS : RegisterClass<"VideoCore4", [i32], 32,
	(add R0, R6, R16, GP)>;