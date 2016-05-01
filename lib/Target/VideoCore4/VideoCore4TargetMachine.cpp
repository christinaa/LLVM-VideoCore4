//===-- VideoCore4TargetMachine.cpp - Define TargetMachine for VideoCore4 -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Top-level implementation for the VideoCore4 target.
//
//===----------------------------------------------------------------------===//

#include "VideoCore4TargetMachine.h"
#include "VideoCore4.h"
#include "llvm/PassManager.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

extern "C" void LLVMInitializeVideoCore4Target() {
  // Register the target.
  RegisterTargetMachine<VideoCore4TargetMachine> X(TheVideoCore4Target);
}

VideoCore4TargetMachine::VideoCore4TargetMachine(const Target &T,
                                         StringRef TT,
                                         StringRef CPU,
                                         StringRef FS,
                                         const TargetOptions &Options,
                                         Reloc::Model RM, CodeModel::Model CM,
                                         CodeGenOpt::Level OL)
  : LLVMTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL),
    Subtarget(TT, CPU, FS),
    // FIXME: Check TargetData string.
    DL("e-p:32:32:32-i8:8:8-i16:16:16-i32:32:32-f32:32:32-n32-S32"),
    InstrInfo(*this), TLInfo(*this), TSInfo(*this),
    FrameLowering(Subtarget) {
	initAsmInfo();
}

namespace {
/// VideoCore4 Code Generator Pass Configuration Options.
class VideoCore4PassConfig : public TargetPassConfig {
public:
  VideoCore4PassConfig(VideoCore4TargetMachine *TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  VideoCore4TargetMachine &getVideoCore4TargetMachine() const {
    return getTM<VideoCore4TargetMachine>();
  }

  virtual bool addInstSelector();
};
} // namespace

TargetPassConfig *VideoCore4TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new VideoCore4PassConfig(this, PM);
}

bool VideoCore4PassConfig::addInstSelector() {
  // Install an instruction selector.
  addPass(createVideoCore4ISelDag(getVideoCore4TargetMachine(), getOptLevel()));
  return false;
}
