
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
	multiclass BCMP<string cond, PatFrag node> {
		def _RR : S32<
					(outs), (ins FR32:$A, FR32:$B, brtarget:$dst),
					"cmp $A, $B\n\tb"#cond#" $dst",
					[(brcond (node (i32 FR32:$A), (i32 FR32:$B)), bb:$dst)]
				>;

		def _RI : S32<
					(outs), (ins FR32:$A, i32imm:$B, brtarget:$dst),
					"cmp $A, $B\n\tb"#cond#" $dst",
					[(brcond (node (i32 FR32:$A), immU6:$B), bb:$dst)]
				>;
	}

	defm BCMP_EQ : BCMP<"eq", seteq>;
	defm BCMP_NE : BCMP<"ne", setne>;
	defm BCMP_GT : BCMP<"gt", setgt>;
	defm BCMP_GE : BCMP<"ge", setge>;
	defm BCMP_LT : BCMP<"lt", setlt>;
	defm BCMP_LE : BCMP<"le", setle>;
	defm BCMP_HI : BCMP<"hi", setugt>;
	defm BCMP_HS : BCMP<"cc", setuge>;
	defm BCMP_LO : BCMP<"cs", setult>;
	defm BCMP_LS : BCMP<"ls", setule>;

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


def CMP_F : S16<
			(outs SR_CLASS:$D), (ins FR32:$A, FR32:$B),
			"cmp $A, $B # fast",
			[]
		>;
def CMP_LI : S48<
			(outs SR_CLASS:$D), (ins GR32:$A, i32imm:$B),
			"cmp $A, $B # long",
			[]
		>;

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
	class JMP_CC<string cond> : S32<
		(outs), (ins SR_CLASS:$CC, brtarget:$dst),
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

multiclass BRCOND<Instruction instr, PatFrag node> {
	def : Pat<
		(brcond (node (i32 FR32:$A), (i32 FR32:$B)), bb:$dst),
		(instr (CMP_F (i32 FR32:$A), (i32 FR32:$B)), bb:$dst)
	>;
	def : Pat<
		(brcond (node (i32 FR32:$A), (i32 imm:$B)), bb:$dst),
		(instr (CMP_LI (i32 FR32:$A), (i32 imm:$B)), bb:$dst)
	>;
}

defm COND_EQ : BRCOND<JMP_CC_EQ, seteq>;
defm COND_NE : BRCOND<JMP_CC_NE, setne>;
defm COND_GT : BRCOND<JMP_CC_GT, setgt>;
defm COND_GE : BRCOND<JMP_CC_GE, setge>;
defm COND_LT : BRCOND<JMP_CC_LT, setlt>;
defm COND_LE : BRCOND<JMP_CC_LE, setle>;
defm COND_HI : BRCOND<JMP_CC_HI, setugt>;
defm COND_HS : BRCOND<JMP_CC_HS, setuge>;
defm COND_LO : BRCOND<JMP_CC_LO, setult>;
defm COND_LS : BRCOND<JMP_CC_LS, setule>;
