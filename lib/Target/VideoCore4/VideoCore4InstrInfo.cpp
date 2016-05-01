//===-- VideoCore4InstrInfo.cpp - VideoCore4 Instruction Information -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the VideoCore4 implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "VideoCore4InstrInfo.h"
#include "VideoCore4.h"
#include "VideoCore4MachineFunctionInfo.h"
#include "VideoCore4TargetMachine.h"
#include "llvm/IR/Function.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#define GET_INSTRINFO_CTOR_DTOR
#include "VideoCore4GenInstrInfo.inc"

using namespace llvm;

VideoCore4InstrInfo::VideoCore4InstrInfo(VideoCore4TargetMachine &tm)
  : VideoCore4GenInstrInfo(VideoCore4::ADJCALLSTACKDOWN, VideoCore4::ADJCALLSTACKUP),
    RI(tm, *this) {}

void VideoCore4InstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                    MachineBasicBlock::iterator I, DebugLoc DL,
                                    unsigned DestReg, unsigned SrcReg,
                                    bool KillSrc) const {
	if (VideoCore4::FR32RegClass.contains(DestReg, SrcReg))
	{
		BuildMI(MBB, I, DL, get(VideoCore4::MOV_F), DestReg)
			.addReg(SrcReg, getKillRegState(KillSrc));
		return;
	}
	else if (VideoCore4::GR32RegClass.contains(DestReg, SrcReg))
	{
		BuildMI(MBB, I, DL, get(VideoCore4::MOV_R), DestReg)
			.addReg(SrcReg, getKillRegState(KillSrc));
		return;
	}

	llvm_unreachable("Cannot emit physreg copy instruction");
}

void VideoCore4InstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                    unsigned SrcReg, bool isKill, int FI,
                    const TargetRegisterClass *RC,
                    const TargetRegisterInfo *TRI) const {

  DebugLoc DL;
  if (I != MBB.end())
	  DL = I->getDebugLoc();

  MachineFunction *MF = MBB.getParent();
  const MachineFrameInfo &MFI = *MF->getFrameInfo();
  MachineMemOperand *MMO =
    MF->getMachineMemOperand(MachinePointerInfo::getFixedStack(FI),
                             MachineMemOperand::MOStore,
                             MFI.getObjectSize(FI),
                             MFI.getObjectAlignment(FI));

	BuildMI(MBB, I, DL, get(VideoCore4::MEM32_ST_LI))
		.addReg(SrcReg, getKillRegState(isKill))
		.addFrameIndex(FI).addImm(0)
		.addMemOperand(MMO);
}

void VideoCore4InstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                    unsigned DestReg, int FI,
                    const TargetRegisterClass *RC,
                    const TargetRegisterInfo *TRI) const {

  DebugLoc DL;
  if (I != MBB.end())
	  DL = I->getDebugLoc();

  MachineFunction *MF = MBB.getParent();
  const MachineFrameInfo &MFI = *MF->getFrameInfo();
  MachineMemOperand *MMO =
    MF->getMachineMemOperand(MachinePointerInfo::getFixedStack(FI),
                             MachineMemOperand::MOStore,
                             MFI.getObjectSize(FI),
                             MFI.getObjectAlignment(FI));

	BuildMI(MBB, I, DL, get(VideoCore4::MEM32_LD_LI), DestReg)
		.addFrameIndex(FI).addImm(0)
		.addMemOperand(MMO);
}

void VideoCore4InstrInfo::adjustStackPtr(int64_t amount, MachineBasicBlock& MBB,
			MachineBasicBlock::iterator I) const {
	DebugLoc DL = I != MBB.end() ? I->getDebugLoc() : DebugLoc();

	if (amount < 0) {
#if defined(USING_VASM)
		BuildMI(MBB, I, DL, get(VideoCore4::MOV_LI), VideoCore4::R15)
			.addImm(-amount);
		BuildMI(MBB, I, DL, get(VideoCore4::SUB_F_RR), VideoCore4::SP)
			.addReg(VideoCore4::SP)
			.addReg(VideoCore4::R15);
#else
		BuildMI(MBB, I, DL, get(VideoCore4::SUB_F_RI), VideoCore4::SP)
			.addReg(VideoCore4::SP)
			.addImm(-amount);
#endif
	}
	else if (amount > 0) {
#if defined(USING_VASM)
		BuildMI(MBB, I, DL, get(VideoCore4::MOV_LI), VideoCore4::R15)
			.addImm(amount);
		BuildMI(MBB, I, DL, get(VideoCore4::ADD_F_RR), VideoCore4::SP)
			.addReg(VideoCore4::SP)
			.addReg(VideoCore4::R15);
#else
		BuildMI(MBB, I, DL, get(VideoCore4::ADD_F_RI), VideoCore4::SP)
			.addReg(VideoCore4::SP)
			.addImm(amount);
#endif
	}
	else
	{
		/* Do nothing if we're adjusting the stack by zero */
	}
}

bool VideoCore4InstrInfo::AnalyzeBranch(MachineBasicBlock &MBB, MachineBasicBlock *&TBB,
                          MachineBasicBlock *&FBB,
                          SmallVectorImpl<MachineOperand> &Cond,
                          bool AllowModify) const
{
	MachineBasicBlock::iterator I = MBB.end();
	MachineInstr *LastInst = I;

	//LastInst->getOpcode()

	//errs() << "AnalyzeBranch: AllowModify = " << AllowModify << "\n";
	//MBB.dump();



	return true;
}

unsigned VideoCore4InstrInfo::InsertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                             MachineBasicBlock *FBB,
                             ArrayRef<MachineOperand> Cond,
                             DebugLoc DL) const
{
	llvm_unreachable("InsertBranch");
}

unsigned VideoCore4InstrInfo::RemoveBranch(MachineBasicBlock &MBB) const
{
	llvm_unreachable("RemoveBranch");
}

