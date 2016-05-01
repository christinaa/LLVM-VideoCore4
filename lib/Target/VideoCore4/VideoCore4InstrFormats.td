//==- VideoCore4InstrFormats.td - VideoCore4 Instruction Formats -*- tablegen -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source 
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// Generic VideoCore4 instruction
class VideoCore4Inst<dag outs, dag ins,
				 string asmstr, list<dag> pattern> : Instruction
{
	field bits<80> Inst;

	let Namespace = "VideoCore4";

	dag OutOperandList = outs;
	dag InOperandList = ins;

	let AsmString = asmstr;
	let Pattern = pattern;

	field bits<80> SoftFail = 0;
}

// 48-bit VC4 instruction
class S48<dag outs, dag ins, string asmstr, list<dag> pattern> :
		VideoCore4Inst<outs, ins, asmstr, pattern> {
	let Size = 6;
}

// 32-bit VC4 instruction
class S32<dag outs, dag ins, string asmstr, list<dag> pattern> :
		VideoCore4Inst<outs, ins, asmstr, pattern> {
	let Size = 4;
}

// 16-bit VC4 instruction
class S16<dag outs, dag ins, string asmstr, list<dag> pattern> :
		VideoCore4Inst<outs, ins, asmstr, pattern> {
	let Size = 2;
}

