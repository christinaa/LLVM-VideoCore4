//==- VideoCore4TargetMachine.h - Define TargetMachine for VideoCore4 -*- C++ -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the VideoCore4 specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//


#ifndef LLVM_TARGET_VIDEOCORE4_TARGETMACHINE_H
#define LLVM_TARGET_VIDEOCORE4_TARGETMACHINE_H

#include "VideoCore4InstrInfo.h"
#include "VideoCore4ISelLowering.h"
#include "VideoCore4FrameLowering.h"
#include "VideoCore4SelectionDAGInfo.h"
#include "VideoCore4RegisterInfo.h"
#include "VideoCore4Subtarget.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

/// VideoCore4TargetMachine
///
class VideoCore4TargetMachine : public LLVMTargetMachine {
  VideoCore4Subtarget        Subtarget;
  const DataLayout           DL;       // Calculates type size & alignment
  VideoCore4InstrInfo        InstrInfo;
  VideoCore4TargetLowering   TLInfo;
  VideoCore4SelectionDAGInfo TSInfo;
  VideoCore4FrameLowering    FrameLowering;

public:
  VideoCore4TargetMachine(const Target &T, StringRef TT,
                        StringRef CPU, StringRef FS, const TargetOptions &Options,
                        Reloc::Model RM, CodeModel::Model CM,
                        CodeGenOpt::Level OL);

  virtual const TargetFrameLowering *getFrameLowering() const {
    return &FrameLowering;
  }
  virtual const VideoCore4InstrInfo *getInstrInfo() const  { return &InstrInfo; }
  virtual const DataLayout *getDataLayout() const     { return &DL;}
  virtual const VideoCore4Subtarget *getSubtargetImpl() const { return &Subtarget; }

  virtual const TargetRegisterInfo *getRegisterInfo() const {
    return &InstrInfo.getRegisterInfo();
  }

  virtual const VideoCore4TargetLowering *getTargetLowering() const {
    return &TLInfo;
  }

  virtual const VideoCore4SelectionDAGInfo* getSelectionDAGInfo() const {
    return &TSInfo;
  }

  virtual TargetPassConfig *createPassConfig(PassManagerBase &PM);
}; // VideoCore4TargetMachine.

} // end namespace llvm

#endif // LLVM_TARGET_VIDEOCORE4_TARGETMACHINE_H
