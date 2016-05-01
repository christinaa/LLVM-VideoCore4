//= VideoCore4InstPrinter.h - Convert VideoCore4 MCInst to asm syntax -*- C++ -*--//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This class prints a VideoCore4 MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#ifndef VIDEOCORE4INSTPRINTER_H
#define VIDEOCORE4INSTPRINTER_H

#include "llvm/MC/MCInstPrinter.h"

namespace llvm {
  class MCOperand;

  class VideoCore4InstPrinter : public MCInstPrinter {
  public:
    VideoCore4InstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                        const MCRegisterInfo &MRI)
      : MCInstPrinter(MAI, MII, MRI) {}

    void printInst(const MCInst *MI, raw_ostream &O, StringRef Annot);
    void printOperand(const MCInst *MI, unsigned OpNo,
                      raw_ostream &O, const char *Modifier = 0);
    void printAddrOperand(const MCInst *MI, unsigned OpNo,
                      raw_ostream &O, const char *Modifier = 0);
		void printStacked(const MCInst *MI, unsigned OpNo,
                      raw_ostream &O, const char *Modifier = 0);

    // Autogenerated by tblgen.
    void printInstruction(const MCInst *MI, raw_ostream &O);
    static const char *getRegisterName(unsigned RegNo);
  };
}

#endif