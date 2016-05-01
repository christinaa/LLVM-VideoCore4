//===-- VideoCore4SelectionDAGInfo.cpp - VideoCore4 SelectionDAG Info ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the VideoCore4SelectionDAGInfo class.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "openrisc-selectiondag-info"
#include "VideoCore4TargetMachine.h"
using namespace llvm;

VideoCore4SelectionDAGInfo::VideoCore4SelectionDAGInfo(const VideoCore4TargetMachine &TM)
  : TargetSelectionDAGInfo(TM) {
}

VideoCore4SelectionDAGInfo::~VideoCore4SelectionDAGInfo() {
}
