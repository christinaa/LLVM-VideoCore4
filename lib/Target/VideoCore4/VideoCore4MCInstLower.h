//===-- VideoCore4MCInstLower.h - Lower MachineInstr to MCInst ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef VIDEOCORE4_MCINSTLOWER_H
#define VIDEOCORE4_MCINSTLOWER_H

#include "llvm/Support/Compiler.h"

namespace llvm {
  class AsmPrinter;
  class MCContext;
  class MCInst;
  class MCOperand;
  class MCSymbol;
  class MachineInstr;
  class MachineModuleInfoMachO;
  class MachineOperand;
  class Mangler;

  /// VideoCore4MCInstLower - This class is used to lower an MachineInstr
  /// into an MCInst.
class LLVM_LIBRARY_VISIBILITY VideoCore4MCInstLower {
  MCContext &Ctx;
	AsmPrinter &AP;

public:
  VideoCore4MCInstLower(MCContext &ctx, Mangler &mang, AsmPrinter &printer)
    : Ctx(ctx), AP(printer) {}
  void Lower(const MachineInstr *MI, MCInst &OutMI) const;
};

}

#endif
