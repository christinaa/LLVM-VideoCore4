//=- VideoCore4RegisterInfo.h - VideoCore4 Register Information Impl -*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the VideoCore4 implementation of the MRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TARGET_VIDEOCORE4REGISTERINFO_H
#define LLVM_TARGET_VIDEOCORE4REGISTERINFO_H

#include "llvm/Target/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "VideoCore4GenRegisterInfo.inc"

namespace llvm {

class TargetInstrInfo;
class VideoCore4TargetMachine;

struct VideoCore4RegisterInfo : public VideoCore4GenRegisterInfo {
public:
  VideoCore4RegisterInfo(VideoCore4TargetMachine &tm, const TargetInstrInfo &tii);

  const uint16_t *getCalleeSavedRegs(const MachineFunction *MF = 0) const;

  BitVector getReservedRegs(const MachineFunction &MF) const;
  const TargetRegisterClass* getPointerRegClass(const MachineFunction &MF,
	  unsigned Kind = 0) const;

  void eliminateFrameIndex(MachineBasicBlock::iterator II,
                           int SPAdj, unsigned FIOperandNum,
						   RegScavenger *RS = NULL) const;

  unsigned getFrameRegister(const MachineFunction &MF) const;
};

} // end namespace llvm

#endif // LLVM_TARGET_VIDEOCORE4REGISTERINFO_H
