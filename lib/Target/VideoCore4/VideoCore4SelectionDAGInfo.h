//==-- VideoCore4SelectionDAGInfo.h - VideoCore4 SelectionDAG Info ---*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the VideoCore4 subclass for TargetSelectionDAGInfo.
//
//===----------------------------------------------------------------------===//

#ifndef VIDEOCORE4SELECTIONDAGINFO_H
#define VIDEOCORE4SELECTIONDAGINFO_H

#include "llvm/Target/TargetSelectionDAGInfo.h"

namespace llvm {

class VideoCore4TargetMachine;

class VideoCore4SelectionDAGInfo : public TargetSelectionDAGInfo {
public:
  explicit VideoCore4SelectionDAGInfo(const VideoCore4TargetMachine &TM);
  ~VideoCore4SelectionDAGInfo();
};

}

#endif // VIDEOCORE4SELECTIONDAGINFO_H
