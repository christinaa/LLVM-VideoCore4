//===-- VideoCore4Subtarget.cpp - VideoCore4 Subtarget Information -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the VideoCore4 specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "VideoCore4Subtarget.h"
#include "VideoCore4.h"
#include "llvm/Support/TargetRegistry.h"

#define DEBUG_TYPE "vc4-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "VideoCore4GenSubtargetInfo.inc"

using namespace llvm;

void VideoCore4Subtarget::anchor() { }

VideoCore4Subtarget::VideoCore4Subtarget(const std::string &TT,
                                     const std::string &CPU,
                                     const std::string &FS) :
  VideoCore4GenSubtargetInfo(TT, CPU, FS) {
  std::string CPUName = "generic";

  // Parse features string.
  ParseSubtargetFeatures(CPUName, FS);
}
