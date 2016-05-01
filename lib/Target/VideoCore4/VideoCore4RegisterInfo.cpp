//===-- VideoCore4RegisterInfo.cpp - VideoCore4 Register Information ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the VideoCore4 implementation of the TargetRegisterInfo
// class.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "openrisc-reg-info"

#include "VideoCore4RegisterInfo.h"
#include "VideoCore4.h"
#include "VideoCore4MachineFunctionInfo.h"
#include "VideoCore4TargetMachine.h"
#include "llvm/IR/Function.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/Support/ErrorHandling.h"

#define GET_REGINFO_TARGET_DESC
#include "VideoCore4GenRegisterInfo.inc"

using namespace llvm;

VideoCore4RegisterInfo::VideoCore4RegisterInfo(VideoCore4TargetMachine &tm,
                                           const TargetInstrInfo &tii)
  : VideoCore4GenRegisterInfo(VideoCore4::PC) {
}

const uint16_t *
VideoCore4RegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  const TargetFrameLowering *TFI = MF->getTarget().getFrameLowering();

  return (TFI->hasFP(*MF) ? CSR_FP_SaveList : CSR_SaveList);
}


BitVector
VideoCore4RegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  const TargetFrameLowering *TFI = MF.getTarget().getFrameLowering();

  Reserved.set(VideoCore4::SP);
  Reserved.set(VideoCore4::PC);
  Reserved.set(VideoCore4::GP);
  //Reserved.set(VideoCore4::SR);
  Reserved.set(VideoCore4::ESP);

  // Mark frame pointer as reserved if needed.
  if (TFI->hasFP(MF))
    Reserved.set(VideoCore4::R6);

  return Reserved;
}

const TargetRegisterClass*
VideoCore4RegisterInfo::getPointerRegClass(const MachineFunction& MF,
		unsigned Kind) const {
  assert(0 && "Unimplemented");
}


void
VideoCore4RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                      int SPAdj, unsigned FIOperandNum,
																			RegScavenger *RS) const {
  assert(SPAdj == 0 && "Unexpected");

  MachineInstr &MI = *II;
  MachineFunction &MF = *MI.getParent()->getParent();

  int frameIndex = MI.getOperand(FIOperandNum).getIndex();
	uint64_t stackSize = MF.getFrameInfo()->getStackSize();
	int64_t spOffset = MF.getFrameInfo()->getObjectOffset(frameIndex);

	unsigned reg = getFrameRegister(MF);
	int64_t offset = spOffset + (int64_t)stackSize;

	MI.getOperand(FIOperandNum).ChangeToRegister(reg, false, false, true);
	MI.getOperand(FIOperandNum + 1).ChangeToImmediate(offset);
}

unsigned
VideoCore4RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const TargetFrameLowering *TFI = MF.getTarget().getFrameLowering();

  return VideoCore4::SP;
}
