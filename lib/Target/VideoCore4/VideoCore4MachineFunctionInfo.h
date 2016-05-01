//===- VideoCore4MachineFuctionInfo.h - VideoCore4 machine func info -*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares VideoCore4-specific per-machine-function information.
//
//===----------------------------------------------------------------------===//

#ifndef VIDEOCORE4MACHINEFUNCTIONINFO_H
#define VIDEOCORE4MACHINEFUNCTIONINFO_H

#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

/// VideoCore4MachineFunctionInfo - This class is derived from MachineFunction and
/// contains private VideoCore4 target-specific information for each MachineFunction.
class VideoCore4MachineFunctionInfo : public MachineFunctionInfo {
	

  virtual void anchor();
public:
	int VarArgsFrameIndex;
	
  VideoCore4MachineFunctionInfo() : VarArgsFrameIndex(0) {}

  explicit VideoCore4MachineFunctionInfo(MachineFunction &MF) {}
};

} // End llvm namespace

#endif
