//===-- VideoCore4InstPrinter.cpp - Convert VideoCore4 MCInst to asm syntax ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This class prints an VideoCore4 MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "asm-printer"
#include "VideoCore4.h"
#include "VideoCore4InstPrinter.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
using namespace llvm;


// Include the auto-generated portion of the assembly writer.
#include "VideoCore4GenAsmWriter.inc"

void VideoCore4InstPrinter::printInst(const MCInst *MI, raw_ostream &O,
                                  StringRef Annot) {
  printInstruction(MI, O);
  printAnnotation(O, Annot);
}

void VideoCore4InstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                       raw_ostream &O, const char *Modifier) {
  assert((Modifier == 0 || Modifier[0] == 0) && "No modifiers supported");

  const MCOperand &Op = MI->getOperand(OpNo);
  
  if (Op.isReg()) {
    O << getRegisterName(Op.getReg());
  }
  else if (Op.isImm()) {
    O << Op.getImm();
	}
  else if (Op.isExpr()) {
		O << *Op.getExpr();
  } else
    assert(0 && "Unknown operand in printOperand");
}

void VideoCore4InstPrinter::printStacked(const MCInst *MI, unsigned OpNo,
                                       raw_ostream &O, const char *Modifier) {
  assert((Modifier == 0 || Modifier[0] == 0) && "No modifiers supported");
  const MCOperand& Base = MI->getOperand(OpNo);
  const MCOperand& Offset = MI->getOperand(OpNo+1);

	O << Offset.getImm() << " (" << getRegisterName(Base.getReg()) << ")";
}
