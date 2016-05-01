//===-- VideoCore4MCAsmInfo.h - VideoCore4 asm properties ----------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source 
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the VideoCore4MCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef VIDEOCORE4TARGETASMINFO_H
#define VIDEOCORE4TARGETASMINFO_H

#include "llvm/MC/MCAsmInfo.h"

namespace llvm {
  class StringRef;

  class VideoCore4MCAsmInfo : public MCAsmInfo {
    virtual void anchor();
  public:
    explicit VideoCore4MCAsmInfo(StringRef TT);
  };

} // namespace llvm

#endif
