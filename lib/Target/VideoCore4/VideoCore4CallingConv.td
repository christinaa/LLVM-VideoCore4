//==- VideoCore4CallingConv.td - Calling Conv. for VideoCore4 -*- tablegen -*---==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// This describes the calling conventions for VideoCore4 architecture.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// VideoCore4 Return Value Calling Convention
//===----------------------------------------------------------------------===//
def RetCC_VideoCore4 : CallingConv<[
  // i32 and f32 are returned in registers R0, R1
  CCIfType<[i32, f32], CCAssignToReg<[R0, R1]>>
]>;

//===----------------------------------------------------------------------===//
// VideoCore4 Argument Calling Conventions
//===----------------------------------------------------------------------===//
def CC_VideoCore4 : CallingConv<[
  // Promote i8 arguments to i32.
  CCIfType<[i8], CCPromoteToType<i32>>,
  // Promote i8 arguments to i32.
  CCIfType<[i16], CCPromoteToType<i32>>,

	// The first 6 integer or float arguments of non-varargs functions are passed
	// in integer registers.
  CCIfType<[i32, f32], CCAssignToReg<[R0, R1, R2, R3, R4, R5]>>,

  // Integer or float values get stored in stack slots that are 4 bytes in
  // size and 4-byte aligned.
  CCIfType<[i32, f32], CCAssignToStack<4, 4>>
]>;

//===----------------------------------------------------------------------===//
// Callee-saved register lists.
//===----------------------------------------------------------------------===//

def CSR: CalleeSavedRegs<(add (sequence "R%u", 6, 23), LR, R27, R29)>;
def CSR_FP: CalleeSavedRegs<(sub CSR, R6)>;
