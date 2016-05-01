//=-- VideoCore4MCInstLower.cpp - Convert VideoCore4 MachineInstr to an MCInst --=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains code to lower VideoCore4 MachineInstrs to their corresponding
// MCInst records.
//
//===----------------------------------------------------------------------===//

#include "VideoCore4.h"
#include "VideoCore4AsmPrinter.h"
#include "VideoCore4MCInstLower.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/IR/Mangler.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/ADT/SmallString.h"
using namespace llvm;

void VideoCore4MCInstLower::Lower(const MachineInstr *MI, MCInst &OutMI) const {
  OutMI.setOpcode(MI->getOpcode());
	MCContext& MC = AP.OutContext;

  for (unsigned i = 0, e = MI->getNumOperands(); i != e; ++i) {
    const MachineOperand &MO = MI->getOperand(i);

    MCOperand MCOp;
    switch (MO.getType()) {
			default:
				MI->dump();
				llvm_unreachable("unknown operand type");

			case MachineOperand::MO_Register:
				// Ignore all implicit register operands.
				if (MO.isImplicit()) continue;
				MCOp = MCOperand::CreateReg(MO.getReg());
				break;

			case MachineOperand::MO_Immediate:
				MCOp = MCOperand::CreateImm(MO.getImm());
				break;

			case MachineOperand::MO_FPImmediate:
			{
				APFloat val = MO.getFPImm()->getValueAPF();
				//MCOp = MCOperand::CreateFPImm(MO.getFPImm());
				MCOp = MCOperand::CreateImm(*val.bitcastToAPInt().getRawData());
				break;
			}

			case MachineOperand::MO_MachineBasicBlock:
			{
				const MCSymbol* symbol = MO.getMBB()->getSymbol();
				const MCSymbolRefExpr* MCSym = MCSymbolRefExpr::Create(symbol, MC);
				//const MCExpr* ME = MCSymbolRefExpr::Create(symbol, MCSymbolRefExpr::VK_None, MC);
				MCOp = MCOperand::CreateExpr(MCSym);
				break;
			}

			case MachineOperand::MO_GlobalAddress:
			{
				const MCSymbol* symbol = AP.getSymbol(MO.getGlobal());
				const MCSymbolRefExpr* MCSym = MCSymbolRefExpr::Create(symbol, MC);
				//const MCExpr* ME = MCSymbolRefExpr::Create(symbol, MCSymbolRefExpr::VK_None, MC);
				MCOp = MCOperand::CreateExpr(MCSym);
				break;
			}

			case MachineOperand::MO_RegisterMask:
				continue;
    }

    OutMI.addOperand(MCOp);
  }
}
