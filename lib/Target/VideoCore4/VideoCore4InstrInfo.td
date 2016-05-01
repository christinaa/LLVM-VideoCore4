//===-- VideoCore4InstrInfo.td - VideoCore4 Instruction defs ---*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file describes the VideoCore4 instructions in TableGen format.
//
//===----------------------------------------------------------------------===//

include "VideoCore4InstrFormats.td"

def retflag       : SDNode<"VideoCore4ISD::RET_FLAG", SDTNone,
                           [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def callseq_start : SDNode<"ISD::CALLSEQ_START",
			SDCallSeqStart<[SDTCisVT<0, i32>]>,
			[SDNPHasChain, SDNPOutGlue]
		>;
def callseq_end : SDNode<"ISD::CALLSEQ_END",
			SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>,
			[SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]
		>;
def call : SDNode<"VideoCore4ISD::CALL",
			SDTypeProfile<0, 1, [SDTCisVT<0, i32>]>,
			[SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPVariadic]
		>;

//===----------------------------------------------------------------------===//
// VideoCore4 Operand Definitions.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// VideoCore4 Complex Pattern Definitions.
//===----------------------------------------------------------------------===//

// An address of something in memory.
//def addr : ComplexPattern<iPTR, 1, "SelectAddr", [], []>;

def GlobalWrapper : SDNode<"VideoCore4ISD::GLOBAL", SDTIntUnaryOp>;

def VideoCoreBR_JT : SDNode<"VideoCore4ISD::BR_JT",
						SDTypeProfile<0, 2, [SDTCisVT<0, i32>, SDTCisVT<1, i32>]>,
                        [SDNPHasChain]>;

// An address of something on the stack.
def stacked : Operand<i32>,
		ComplexPattern<i32, 2, "SelectStacked", [frameindex], []> {
	let PrintMethod = "printStacked";
	let MIOperandInfo = (ops GR32, imm);
}

def addrRegImm :
  ComplexPattern<iPTR, 2, "SelectStacked", [frameindex]>;

def InlineJT : Operand<i32> {
//  let PrintMethod = "printInlineJT";
}

//===----------------------------------------------------------------------===//
// Pattern Fragments
//===----------------------------------------------------------------------===//
def LO16 : SDNodeXForm<imm, [{
  // Transformation function: return low 16 bits.
  return getI16Imm(N->getZExtValue() & 0xFFFFULL);
}]>;

def HI16 : SDNodeXForm<imm, [{
  // Transformation function: return bits 16-31.
  return getI16Imm(N->getZExtValue() >> 16);
}]>;

def i32lo16 : PatLeaf<(i32 imm), [{
  // i32lo16 predicate - true if the 32-bit immediate has only rightmost 16
  // bits set.
  return ((N->getZExtValue() & 0FFFFULL) == N->getZExtValue());
}], LO16>;

def i32hi16 : PatLeaf<(i32 imm), [{
  // i32lo16 predicate - true if the 32-bit immediate has only leftmost 16
  // bits set.
  return ((N->getZExtValue() & 0xFFFF0000ULL) == N->getZExtValue());
}], HI16>;

def immSExt16 : PatLeaf<(i32 imm), [{
  // immSExt16 predicate - true if the immediate fits in a 16-bit sign extended
  // field.
  uint64_t val = N->getZExtValue();
  return ((int64_t)val == (int16_t)val);
}]>;

// true if the immediate fits in a 5-bit unsigned field.
def immU5 : PatLeaf<
			(i32 imm),
			[{
				return isUInt<5>(N->getZExtValue());
			}]
		>;

// true if the immediate fits in a 6-bit signed field.
def immS6 : PatLeaf<
			(i32 imm),
			[{
				return isInt<6>(N->getSExtValue());
			}]
		>;

// true if the immediate fits in a 6-bit unsigned field.
def immU6 : PatLeaf<
			(i32 imm),
			[{
				return isUInt<6>(N->getZExtValue());
			}]
		>;

// true if the immediate fits in a 12-bit unsigned field.
def immU12 : PatLeaf<
			(i32 imm),
			[{
				return isUInt<12>(N->getZExtValue());
			}]
		>;

// true if the immediate fits in a 16-bit unsigned field.
def immU16 : PatLeaf<
			(i32 imm),
			[{
				return isUInt<16>(N->getZExtValue());
			}]
		>;

def brtarget : Operand<OtherVT>;
def calltarget : Operand<i32>;

//===----------------------------------------------------------------------===//
// Instruction list..
//===----------------------------------------------------------------------===//

let neverHasSideEffects = 1 in
	def NOP : VideoCore4Inst<(outs), (ins),
													"nop", []>;

let isReturn = 1, isTerminator = 1, isBarrier = 1 in {
  def RET : S32<
				(outs), (ins variable_ops),
				"b lr",
				[(retflag)]
			>;
}

/* --- Calls ---------------------------------------------------------------- */

// ADJCALLSTACKDOWN/UP implicitly use/def SP because they may be expanded into
// a stack adjustment and the codegen must know that they may modify the stack
// pointer before prolog-epilog rewriting occurs.
// Pessimistically assume ADJCALLSTACKDOWN / ADJCALLSTACKUP will become
// sub / add which can clobber SP.
let Defs = [SP], Uses = [SP] in {
	def ADJCALLSTACKDOWN : VideoCore4Inst<
				(outs), (ins i32imm:$d),
				"#ADJCALLSTACKDOWN $d",
				[(callseq_start timm:$d)]
			>;

	def ADJCALLSTACKUP : VideoCore4Inst<
				(outs), (ins i32imm:$a, i32imm:$b),
				"#ADJCALLSTACKUP $a $b",
				[(callseq_end timm:$a, timm:$b)]
			>;
}

let Defs = [R0, R1, R2, R3, R4, R5, LR], isCall = 1 in {
	def CALL : S32<
				(outs), (ins calltarget:$D),
				"bl $D",
				[(call texternalsym:$D)]
			>;

	def CALL_R : S16<
				(outs), (ins GR32:$D),
				"bl $D",
				[(call GR32:$D)]
			>;
}

////////////////////////////////////////////////////////////////////////////////////
// ALU Instruction Formats
////////////////////////////////////////////////////////////////////////////////////

multiclass ALU1_REG<string op, RegisterClass rc, dag node> {
	let Constraints = "$D = $A" in {
		def _R : S16<
					(outs rc:$D), (ins rc:$A),
					op,
					[(set (i32 rc:$D), node)]
				>;
	}
}

multiclass ALU1_REG_REG<string op, RegisterClass rc, dag node> {
	def _RR : S16<
				(outs rc:$D), (ins rc:$A),
				op,
				[(set (i32 rc:$D), node)]
			>;
}

multiclass ALU1_REG_IMM<string op, RegisterClass rc, dag node> {
	def _RI : S16<
				(outs rc:$D), (ins i32imm:$A),
				op,
				[(set (i32 rc:$D), node)]
			>;
}

multiclass ALU1_F_RI<string op2, string op3, PatFrag node> {
	defm _F : ALU1_REG<op2, FR32, (node FR32:$A)>;
	defm _F : ALU1_REG_REG<op3, FR32, (node FR32:$A)>;

	defm _R : ALU1_REG<op2, GR32, (node GR32:$A)>;
	defm _R : ALU1_REG_REG<op3, GR32, (node GR32:$A)>;
	defm _R : ALU1_REG_IMM<op3, GR32, (node immU5:$A)>;
}

multiclass ALU1_FI_RI<string op2, string op3, PatFrag node> {
	defm _F : ALU1_REG<op2, FR32, (node FR32:$A)>;
	defm _F : ALU1_REG_REG<op3, FR32, (node FR32:$A)>;
	defm _F : ALU1_REG_IMM<op3, FR32, (node immU5:$A)>;

	defm _R : ALU1_REG<op2, GR32, (node GR32:$A)>;
	defm _R : ALU1_REG_REG<op3, GR32, (node GR32:$A)>;
	defm _R : ALU1_REG_IMM<op3, GR32, (node immU5:$A)>;
}

/* --- ALU two-input instruction forms -------------------------------------- */

multiclass ALU2_REG_IMM<string op, RegisterClass rc, dag node> {
	let Constraints = "$D = $A" in {
		def _RI : S16<
				(outs rc:$D), (ins rc:$A, i32imm:$B),
				op,
				[(set (i32 rc:$D), node)]
			>;
	}
}

multiclass ALU2_REG_IMM_PSEUDO<string op, RegisterClass rc> {
	let Constraints = "$D = $A" in {
		def _RI : S16<
				(outs rc:$D), (ins rc:$A, i32imm:$B),
				op,
				[]
			>;
	}
}

multiclass ALU2_REG_REG<string op, RegisterClass rc, dag node> {
	let Constraints = "$D = $A" in {
		def _RR : S16<
				(outs rc:$D), (ins rc:$A, rc:$B),
				op,
				[(set (i32 rc:$D), node)]
			>;
	}
}

multiclass ALU2_REG_REG_C<string op, RegisterClass rc, dag node> {
	let isCommutable = 1 in {
		defm _C : ALU2_REG_REG<op, rc, node>;
	}
}

multiclass ALU2_REG_REG_IMM<string op, RegisterClass rc, dag node> {
	def _RRI : S32<
			(outs rc:$D), (ins rc:$A, i32imm:$B),
			op,
			[(set (i32 rc:$D), node)]
		>;
}

multiclass ALU2_REG_REG_REG<string op, RegisterClass rc, dag node> {
	def _RRR : S32<
			(outs rc:$D), (ins rc:$A, rc:$B),
			op,
			[(set (i32 rc:$D), node)]
		>;
}

multiclass ALU2_REG_REG_REG_C<string op, RegisterClass rc, dag node> {
	let isCommutable = 1 in {
		defm _C : ALU2_REG_REG_REG<op, rc, node>;
	}
}

multiclass ALU2_FI_RI<string op2, string op3, PatFrag node> {
	defm _F : ALU2_REG_IMM<op2#" # short", FR32, (node (i32 FR32:$A), (immU5:$B))>;
	defm _F : ALU2_REG_REG<op2#" # short", FR32, (node (i32 FR32:$A), (i32 FR32:$B))>;
	defm _R : ALU2_REG_REG_IMM<op3#" # medium", GR32, (node (i32 GR32:$A), (immU5:$B))>;
	defm _R : ALU2_REG_REG_REG<op3#" # medium", GR32, (node (i32 GR32:$A), (i32 GR32:$B))>;
	defm _R : ALU2_REG_IMM<op2#" # long", GR32, (node (i32 GR32:$A), (imm:$B))>;
}
	
multiclass ALU2_F_RI<string op2, string op3, PatFrag node> {
	defm _F : ALU2_REG_REG<op2#" # short", FR32, (node (i32 FR32:$A), (i32 FR32:$B))>;
	defm _F : ALU2_REG_REG_IMM<op3#" # medium", GR32, (node (i32 GR32:$A), (immU5:$B))>;
	defm _R : ALU2_REG_REG_REG<op3#" # medium", GR32, (node (i32 GR32:$A), (i32 GR32:$B))>;
	defm _R : ALU2_REG_IMM<op2#" # long", GR32, (node (i32 GR32:$A), (imm:$B))>;
}

/* --- ALU instructions ----------------------------------------------------- */

defm ADD : ALU2_FI_RI<
		"add $D, $B",
		"add $D, $A, $B",
		PatFrag<
			(ops node:$inA, node:$inB),
			(add node:$inA, node:$inB)
		>
	>;
defm ADD_3_LONG : ALU2_REG_REG_IMM<
		"add $D, $A, $B # long",
		GR32,
		(add (i32 GR32:$A), imm:$B)
	>;

defm MUL : ALU2_FI_RI<
		"mul $D, $B",
		"mul $D, $A, $B",
		PatFrag<
			(ops node:$inA, node:$inB),
			(mul node:$inA, node:$inB)
		>
	>;

defm XOR : ALU2_F_RI<
		"eor $D, $B",
		"eor $D, $A, $B",
		PatFrag<
			(ops node:$inA, node:$inB),
			(xor node:$inA, node:$inB)
		>
	>;

defm SUB : ALU2_FI_RI<
		"sub $D, $B",
		"sub $D, $A, $B",
		PatFrag<
			(ops node:$inA, node:$inB),
			(sub node:$inA, node:$inB)
		>
	>;

defm AND : ALU2_F_RI<
		"and $D, $B",
		"and $D, $A, $B",
		PatFrag<
			(ops node:$inA, node:$inB),
			(and node:$inA, node:$inB)
		>
	>;

defm NOT : ALU1_F_RI<
		"not $D, $D",
		"not $D, $A",
		PatFrag<
			(ops node:$in),
			(xor node:$in, -1)
		>
	>;

defm RSUB : ALU2_FI_RI<
		"rsub $D, $B",
		"rsub $D, $A, $B",
		PatFrag<
			(ops node:$inA, node:$inB),
			(sub node:$inB, node:$inA)
		>
	>;

defm OR : ALU2_F_RI<
		"or $D, $B",
		"or $D, $A, $B",
		PatFrag<
			(ops node:$inA, node:$inB),
			(or node:$inA, node:$inB)
		>
	>;

defm SHR : ALU2_FI_RI<
		"lsr $D, $B",
		"lsr $D, $A, $B",
		PatFrag<
			(ops node:$inA, node:$inB),
			(srl node:$inA, node:$inB)
		>
	>;

defm SHL : ALU2_FI_RI<
		"lsl $D, $B",
		"lsl $D, $A, $B",
		PatFrag<
			(ops node:$inA, node:$inB),
			(shl node:$inA, node:$inB)
		>
	>;

defm ASR : ALU2_FI_RI<
		"asr $D, $B",
		"asr $D, $A, $B",
		PatFrag<
			(ops node:$inA, node:$inB),
			(sra node:$inA, node:$inB)
		>
	>;

defm SEXT8 : ALU1_FI_RI<
		"exts $D, 8",
		"exts $D, $A, 8",
		PatFrag<
			(ops node:$in),
			(sext_inreg node:$in, i8)
		>
	>;

defm SEXT16 : ALU1_FI_RI<
		"exts $D, 16",
		"exts $D, $A, 16",
		PatFrag<
			(ops node:$in),
			(sext_inreg node:$in, i16)
		>
	>;

defm NEG : ALU1_F_RI<
		"neg $D, $D",
		"neg $D, $A",
		PatFrag<
			(ops node:$in),
			(sub 0, node:$in)
		>
	>;

multiclass ADDSCALE<int n> {
	defm _ : ALU2_F_RI<
		"addscale $D, $B scale "#n,
		"addscale $D, $A, $B scale "#n,
		PatFrag<
			(ops node:$inA, node:$inB),
			(add node:$inA,
						(shl node:$inB, n))
		>
	>;
}
defm ADDs1 : ADDSCALE<1>;
defm ADDs2 : ADDSCALE<2>;
defm ADDs3 : ADDSCALE<3>; // actually an ALU2_FI_R instruction
defm ADDs4 : ADDSCALE<4>;

defm MULHD_UU_R : ALU2_REG_REG_REG<
	"mulhdu $D, $A, $B",
	GR32,
	(mulhu (i32 GR32:$A), (i32 GR32:$B))
>;

defm MULHD_UU_I : ALU2_REG_REG_IMM<
	"mulhdu $D, $A, $B",
	GR32,
	(mulhu (i32 GR32:$A), immS6:$B)
>;

defm MULHD_SS_R : ALU2_REG_REG_REG<
	"mulhds $D, $A, $B",
	GR32,
	(mulhs (i32 GR32:$A), (i32 GR32:$B))
>;

defm MULHD_SS_I : ALU2_REG_REG_IMM<
	"mulhds $D, $A, $B",
	GR32,
	(mulhs (i32 GR32:$A), immS6:$B)
>;

defm DIV_R : ALU2_REG_REG_REG<
	"divs $D, $A, $B",
	GR32,
	(sdiv (i32 GR32:$A), (i32 GR32:$B))
>;

/*
defm DIV_I : ALU2_REG_REG_IMM<
	"divs $D, $A, $B",
	GR32,
	(sdiv (i32 GR32:$A), immS6:$B)
>;
*/

defm DIVU_R : ALU2_REG_REG_REG<
	"divu $D, $A, $B",
	GR32,
	(udiv (i32 GR32:$A), (i32 GR32:$B))
>;

defm DIVU_I : ALU2_REG_REG_IMM<
	"divu $D, $A, $B",
	GR32,
	(udiv (i32 GR32:$A), immS6:$B)
>;

////////////////////////////////////////////////////////////////////////////////////
// Memory Operations
////////////////////////////////////////////////////////////////////////////////////

/* Small fastreg loads/stores. */

multiclass MEMST_F<string op, ValueType vt, PatFrag stop> {
	def _F : S16<
			(outs), (ins FR32:$S, FR32:$D),
			"st"#op#" $S, ($D)",
			[(stop (vt FR32:$S), (iPTR FR32:$D))]
		>;
}

multiclass MEMLD_F<string op, ValueType vt, PatFrag ldop> {
	def _F : S16<
			(outs FR32:$D), (ins FR32:$S),
			"ld"#op#" $D, ($S)",
			[(set (vt FR32:$D), (ldop (iPTR FR32:$S)))]
		>;
}

multiclass MEM_F<string op, ValueType vt, PatFrag stop, PatFrag ldop> {
	defm _ST : MEMST_F<op, vt, stop>;
	defm _LD : MEMLD_F<op, vt, ldop>;
}

defm MEM8 : MEM_F<"b", i32, truncstorei8, zextloadi8>;
defm MEM8E_LD : MEMLD_F<"b", i32, extloadi8>;

defm MEM16 : MEM_F<"h", i32, truncstorei16, zextloadi16>;
defm MEM16S_LD : MEMLD_F<"cc", i32, sextloadi16>;
defm MEM16E_LD : MEMLD_F<"h", i32, extloadi16>;

defm MEM32 : MEM_F<"", i32, store, load>;
defm MEM32F : MEM_F<"", f32, store, load>;

/* Indexed loads/stores. */

multiclass MEMST_R_I<string op, ValueType vt, PatFrag stop, PatFrag scale> {
	def _R : S32<
			(outs), (ins GR32:$S, GR32:$A, GR32:$B),
			"st"#op#" $S, ($A, $B)",
			[(stop (vt GR32:$S),
				(iPTR (scale (i32 GR32:$A), (i32 GR32:$B))))]
		>;

	def _I : S32<
			(outs), (ins GR32:$S, GR32:$A, i32imm:$B),
			"st"#op#" $S, $B ($A) # s12-bit displacement",
			[(stop (vt GR32:$S),
				(iPTR (add (i32 GR32:$A), immU12:$B)))]
		>;

	def _LI : S32<
			(outs), (ins GR32:$S, IR32:$A, i32imm:$B),
			"st"#op#" $S, $B ($A) # s16-bit displacement",
			[(stop (vt GR32:$S),
				(iPTR (add (i32 IR32:$A), immU16:$B)))]
		>;
}

multiclass MEMLD_R_I<string op, ValueType vt, PatFrag ldop, PatFrag scale> {
	def _R : S32<
			(outs GR32:$D), (ins GR32:$A, GR32:$B),
			"ld"#op#" $D, ($A, $B)",
			[(set (vt GR32:$D),
				(ldop (iPTR (scale (i32 GR32:$A), (i32 GR32:$B)))))]
		>;

	def _I : S32<
			(outs GR32:$D), (ins GR32:$A, i32imm:$B),
			"ld"#op#" $D, $B ($A) # s12-bit displacement",
			[(set (vt GR32:$D),
				(ldop (iPTR (add (i32 GR32:$A), immU12:$B))))]
		>;

	def _LI : S32<
			(outs GR32:$D), (ins IR32:$A, i32imm:$B),
			"ld"#op#" $D, $B ($A) # s16-bit displacement",
			[(set (vt GR32:$D),
				(ldop (iPTR (add (i32 IR32:$A), immU16:$B))))]
		>;
}

multiclass MEM_R_I<string op, ValueType vt, PatFrag stop, PatFrag ldop,
			PatFrag scale> {
	defm _LD : MEMLD_R_I<op, vt, ldop, scale>;
	defm _ST : MEMST_R_I<op, vt, stop, scale>;
}

def memb_frag : PatFrag<
				(ops node:$inA, node:$inB),
				(add node:$inA, node:$inB)
			>;
defm MEM8 : MEM_R_I<"b", i32, truncstorei8, zextloadi8, memb_frag>;
defm MEM8E_LD : MEMLD_R_I<"b", i32, extloadi8, memb_frag>;

def memh_frag : PatFrag<
				(ops node:$inA, node:$inB),
				(add node:$inA, (shl node:$inB, 1))
			>;
defm MEM16 : MEM_R_I<"h", i32, truncstorei16, zextloadi16, memh_frag>;
defm MEM16S_LD : MEMLD_R_I<"cc", i32, sextloadi16, memh_frag>;
defm MEM16E_LD : MEMLD_R_I<"cc", i32, extloadi16, memh_frag>;

defm MEM32 : MEM_R_I<"", i32, store, load,
			PatFrag<
				(ops node:$inA, node:$inB),
				(add node:$inA, (shl node:$inB, 2))
			>
		>;

/* Stack frame loads/stores. */

multiclass MEMFRAME<string op, ValueType vt> {
	def _LD : S32<
			(outs GR32:$D), (ins stacked:$A),
			"ld"#op#" $D, $A",
			[(set (vt GR32:$D), (load (iPTR stacked:$A)))]
		>;

	def _ST : S32<
			(outs), (ins GR32:$A, stacked:$B),
			"st"#op#" $A, $B",
			[(store (vt GR32:$A), (iPTR stacked:$B))]
		>;
}

def PUSH : S32<
			(outs), (ins PUSHPOP_START_REGS:$StartReg, GR32:$EndReg),
			"push $StartReg-$EndReg",
			[]
		>;

def PUSH_LR : S32<
			(outs), (ins PUSHPOP_START_REGS:$StartReg, GR32:$EndReg),
			"push $StartReg-$EndReg, lr",
			[]
		>;

def POP : S32<
			(outs), (ins PUSHPOP_START_REGS:$StartReg, GR32:$EndReg),
			"pop $StartReg-$EndReg",
			[]
		>;

def POP_PC : S32<
			(outs), (ins PUSHPOP_START_REGS:$StartReg, GR32:$EndReg),
			"push $StartReg-$EndReg, pc",
			[]
		>;

defm MEMFRAME : MEMFRAME<"", i32>;
defm MEMFRAMEF : MEMFRAME<"", f32>;

/* --- Moves and specials --------------------------------------------------- */

let isAsCheapAsAMove = 1 in
{
	class MOV<RegisterClass rc, dag node> :
		S16<
			(outs rc:$D), (ins rc:$S),
			"mov $D, $S",
			[(set rc:$D, node)]
		>;

	def MOV_FI : MOV<FR32, (i32 immU5:$S)>;
	def MOV_RI : MOV<GR32, (i32 immU5:$S)>;

	def MOV_F : S16<
				(outs FR32:$D), (ins FR32:$A),
				"mov $D, $A # fast",
				[]
			>;

	def MOV_R : S32<
				(outs GR32:$D), (ins GR32:$A),
				"mov $D, $A # medium",
				[]
			>;

	def MOV_LI : S48<
				(outs GR32:$D), (ins i32imm:$A),
				"mov $D, $A # long",
				[(set GR32:$D, imm:$A)]
			>;

	def MOVF_LI : S48<
				(outs GR32:$D), (ins f32imm:$A),
				"mov $D, $A # float long",
				[(set GR32:$D, fpimm:$A)]
			>;
}

def LEA_PC_LI : S48<
				(outs GR32:$D), (ins i32imm:$A),
				"lea $D, $A(pc) # PCrel load",
				[(set GR32:$D, imm:$A)]
			>
{
	bits<5> D;
	bits<32> A;
	let Inst{47-37} = 0b11100101000;
	let Inst{36-32} = D;
	let Inst{31-0} = A;

	let Uses = [PC];
}

def : Pat<
		(GlobalWrapper tglobaladdr:$addr),
		(LEA_PC_LI tglobaladdr:$addr)
	>;

let isBranch=1, isIndirectBranch=1, isTerminator=1, isBarrier = 1 in
def BR_JT : S48<(outs), (ins InlineJT:$t, GR32:$i),
                            "# BR_JT Pseudo Instruction $i\n$t",
                            [(VideoCoreBR_JT tjumptable:$t, GR32:$i)]>;


// --- Branch instructions -------------------------------------------------- 

let Defs = [SR] in {
	def CMP_F : S16<
				(outs), (ins FR32:$A, FR32:$B),
				"cmp $A, $B # fast",
				[]
			>;
	def CMP_LI : S48<
				(outs), (ins GR32:$A, i32imm:$B),
				"cmp $A, $B # long imm",
				[]
			>;
}

let isTerminator = 1, isBranch = 1 in {
	let isBarrier = 1 in {
		def JMP : S32<
					(outs), (ins brtarget:$dst),
					"b $dst",
					[(br bb:$dst)]
				>;
		
		def JMP_R : S16<
					(outs), (ins GR32:$dst),
					"b $dst",
					[(brind GR32:$dst)]
				>;
	}
	
	/* Generates a 3op compare-and-branch instruction. */
	let Uses = [SR] in {
		class JMP_CC<string cond> : S32<
			(outs), (ins brtarget:$dst),
			"b"#cond#" $dst",
			[]
		>;

		def JMP_CC_EQ : JMP_CC<"eq">;
		def JMP_CC_NE : JMP_CC<"ne">;
		def JMP_CC_GT : JMP_CC<"gt">;
		def JMP_CC_GE : JMP_CC<"ge">;
		def JMP_CC_LT : JMP_CC<"lt">;
		def JMP_CC_LE : JMP_CC<"le">;
		def JMP_CC_HI : JMP_CC<"hi">;
		def JMP_CC_HS : JMP_CC<"cc">;
		def JMP_CC_LO : JMP_CC<"cs">;
		def JMP_CC_LS : JMP_CC<"ls">;
	}

	def BCMP_GEN : S32<
				(outs), (ins FR32:$A, brtarget:$dst),
				"# if ($A) then $dst()\n\tcmp $A, 0\n\tbne $dst",
				[(brcond FR32:$A, bb:$dst)]
			>;
 
	/* Likewise, for floats. */
	multiclass BCMPF<string cond, PatFrag node> {
		def _RR : S32<
					(outs), (ins GR32:$A, GR32:$B, brtarget:$dst),
					"fcmp $A, $B\n\tb"#cond#" $dst",
					[(brcond (node (f32 GR32:$A), (f32 GR32:$B)), bb:$dst)]
				>;
	}

	defm FCMPOEQ : BCMPF<"eq", setoeq>;
	defm FCMPONE : BCMPF<"ne", setone>;
	defm FCMPOGT : BCMPF<"gt", setogt>;
	defm FCMPOGE : BCMPF<"ge", setoge>;
	defm FCMPOLT : BCMPF<"lt", setolt>;
	defm FCMPOLE : BCMPF<"le", setole>;

	defm FCMPUEQ : BCMPF<"eq", setueq>;
	defm FCMPUNE : BCMPF<"ne", setune>;
	defm FCMPUHI : BCMPF<"hi", setugt>;
	defm FCMPUHS : BCMPF<"cc", setuge>;
	defm FCMPULO : BCMPF<"cs", setult>;
	defm FCMPULS : BCMPF<"ls", setule>;
}

// Select and SetCC

multiclass CONDCC<string cond, PatFrag node> {
	let Constraints = "$D = $F" in {
		def _SELECT_RR : S32<
					(outs FR32:$D), (ins FR32:$A, FR32:$B, FR32:$T, FR32:$F),
					"cmp $A, $B\n\tmov"#cond#" $D, $T",
					[(set FR32:$D,
						(select (node (i32 FR32:$A), (i32 FR32:$B)), (i32 FR32:$T), (i32 FR32:$F)))]
				>;

		def _SELECT_RR_2 : S32<
					(outs FR32:$D), (ins FR32:$A, FR32:$B, FR32:$T, FR32:$F),
					"cmp $A, $B\n\tmov"#cond#" $D, $T",
					[(set FR32:$D,
						(select (node (i32 FR32:$A), (i32 FR32:$B)), (f32 FR32:$T), (f32 FR32:$F)))]
				>;
	}

	def _SET_RI : S16<
				(outs FR32:$D), (ins FR32:$A, i32imm:$B),
				"cmp $A, $B\n\tmov $D, 0\n\tmov"#cond#" $D, 1",
				[(set FR32:$D, (node (i32 FR32:$A), immU5:$B))]
			>;

	def _SET_RR : S16<
				(outs FR32:$D), (ins FR32:$A, FR32:$B),
				"cmp $A, $B\n\tmov $D, 0\n\tmov"#cond#" $D, 1",
				[(set FR32:$D, (node (i32 FR32:$A), (i32 FR32:$B)))]
			>;
}

defm CONDEQ : CONDCC<"eq", seteq>;
defm CONDNE : CONDCC<"ne", setne>;
defm CONDGT : CONDCC<"gt", setgt>;
defm CONDGE : CONDCC<"ge", setge>;
defm CONDLT : CONDCC<"lt", setlt>;
defm CONDLE : CONDCC<"le", setle>;
defm CONDHI : CONDCC<"hi", setugt>;
defm CONDHS : CONDCC<"cc", setuge>;
defm CONDLO : CONDCC<"cs", setult>;
defm CONDLS : CONDCC<"ls", setule>;

def SELECT_GEN : S32<
			(outs FR32:$D), (ins FR32:$A, FR32:$B, FR32:$T),
			"# $D = if ($A) then $T else $B \n\tcmp $A, 0\n\tmovne $D, $T\n\tmoveq $D, $B",
			[(set FR32:$D, (select (i32 FR32:$A), (i32 FR32:$B), (i32 FR32:$T)))]
		>;

/* Likewise for float. */

multiclass FCONDCC<string cond, PatFrag node> {
	let Constraints = "$D = $F" in {
		def _SELECT_RR : S16<
					(outs GR32:$D), (ins GR32:$A, GR32:$B, GR32:$T, GR32:$F),
					"fcmp $A, $B\n\tmov"#cond#" $D, $T",
					[(set GR32:$D,
						(select (node (f32 GR32:$A), (f32 GR32:$B)), (i32 GR32:$T), (i32 GR32:$F)))]
				>;

		def _SELECT_RR_F : S16<
					(outs GR32:$D), (ins GR32:$A, GR32:$B, GR32:$T, GR32:$F),
					"fcmp $A, $B\n\tmov"#cond#" $D, $T",
					[(set GR32:$D,
						(select (node (f32 GR32:$A), (f32 GR32:$B)), (f32 GR32:$T), (f32 GR32:$F)))]
				>;
	}

	def _SET_RR : S16<
				(outs GR32:$D), (ins GR32:$A, GR32:$B),
				"fcmp $A, $B\n\tmov $D, #0\n\tmov"#cond#" $D, #1",
				[(set GR32:$D, (node (f32 GR32:$A), (f32 GR32:$B)))]
			>;
}

defm FCONDOEQ : FCONDCC<"eq", setoeq>;
defm FCONDONE : FCONDCC<"ne", setone>;
defm FCONDOGT : FCONDCC<"gt", setogt>;
defm FCONDOGE : FCONDCC<"ge", setoge>;
defm FCONDOLT : FCONDCC<"lt", setolt>;
defm FCONDOLE : FCONDCC<"le", setole>;
defm FCONDUEQ : FCONDCC<"eq", setueq>;
defm FCONDUNE : FCONDCC<"ne", setune>;
defm FCONDUGT : FCONDCC<"gt", setugt>;
defm FCONDUGE : FCONDCC<"ge", setuge>;
defm FCONDULT : FCONDCC<"lt", setult>;
defm FCONDULE : FCONDCC<"le", setule>;

/* --- Floating point arithmetic -------------------------------------------- */

let isCommutable = 1 in {
	def FADDrr : S32<
				(outs GR32:$rD), (ins GR32:$rA, GR32:$rB),
				"fadd {$rD, $rA, $rB}",
				[(set (f32 GR32:$rD), (fadd (f32 GR32:$rA), (f32 GR32:$rB)))]
			>;
}

def FSUBrr : S32<
			(outs GR32:$rD), (ins GR32:$rA, GR32:$rB),
			"fsub {$rD, $rA, $rB}",
			[(set (f32 GR32:$rD), (fsub (f32 GR32:$rA), (f32 GR32:$rB)))]
		>;

def FMULrr : S32<
			(outs GR32:$rD), (ins GR32:$rA, GR32:$rB),
			"fmul {$rD, $rA, $rB}",
			[(set (f32 GR32:$rD), (fmul (f32 GR32:$rA), (f32 GR32:$rB)))]
		>;

def FDIVrr : S32<
			(outs GR32:$rD), (ins GR32:$rA, GR32:$rB),
			"fdiv {$rD, $rA, $rB}",
			[(set (f32 GR32:$rD), (fdiv (f32 GR32:$rA), (f32 GR32:$rB)))]
		>;

//===----------------------------------------------------------------------===//
// Non-Instruction Patterns
