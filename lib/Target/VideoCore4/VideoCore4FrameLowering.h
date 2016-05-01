//- VideoCore4FrameLowering.h - Define frame lowering for VideoCore4 --*- C++ -*--//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//
//===----------------------------------------------------------------------===//

#ifndef VIDEOCORE4_FRAMEINFO_H
#define VIDEOCORE4_FRAMEINFO_H

#include "VideoCore4.h"
#include "VideoCore4Subtarget.h"
#include "llvm/Target/TargetFrameLowering.h"

namespace llvm {
  class VideoCore4Subtarget;

class VideoCore4FrameLowering : public TargetFrameLowering {
protected:
  const VideoCore4Subtarget &STI;
	void determineFrameLayout(MachineFunction& MF) const;

public:
  explicit VideoCore4FrameLowering(const VideoCore4Subtarget &sti)
    : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, 4, 0), STI(sti) {
  }

  /// emitProlog/emitEpilog - These methods insert prolog and epilog code into
  /// the function.
  void emitPrologue(MachineFunction &MF) const;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const;

	int getFrameIndexOffset(const MachineFunction& MF, int FI) const;
  bool hasFP(const MachineFunction &MF) const;

	void eliminateCallFramePseudoInstr(MachineFunction &MF,
			MachineBasicBlock &MBB, MachineBasicBlock::iterator MI) const;

  bool
  spillCalleeSavedRegisters(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MI,
                            const std::vector<CalleeSavedInfo> &CSI,
                            const TargetRegisterInfo *TRI) const;
  bool 
  restoreCalleeSavedRegisters(MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator MI,
                              const std::vector<CalleeSavedInfo> &CSI,
                              const TargetRegisterInfo *TRI) const;
};

} // End llvm namespace

#endif
