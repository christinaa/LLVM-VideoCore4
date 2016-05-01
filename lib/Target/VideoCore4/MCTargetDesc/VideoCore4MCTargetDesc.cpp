//===-- VideoCore4MCTargetDesc.cpp - VideoCore4 Target Descriptions ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides VideoCore4 specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "VideoCore4MCTargetDesc.h"
#include "VideoCore4MCAsmInfo.h"
#include "InstPrinter/VideoCore4InstPrinter.h"
#include "llvm/MC/MCCodeGenInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#include "VideoCore4GenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "VideoCore4GenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "VideoCore4GenRegisterInfo.inc"

using namespace llvm;

static MCInstrInfo *createVideoCore4MCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitVideoCore4MCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createVideoCore4MCRegisterInfo(StringRef TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitVideoCore4MCRegisterInfo(X, VideoCore4::PC);
  return X;
}

static MCSubtargetInfo *createVideoCore4MCSubtargetInfo(StringRef TT, StringRef CPU,
                                                      StringRef FS) {
  MCSubtargetInfo *X = new MCSubtargetInfo();
  InitVideoCore4MCSubtargetInfo(X, TT, CPU, FS);
  return X;
}

static MCCodeGenInfo *createVideoCore4MCCodeGenInfo(StringRef TT, Reloc::Model RM,
                                                  CodeModel::Model CM,
                                                  CodeGenOpt::Level OL) {
  MCCodeGenInfo *X = new MCCodeGenInfo();
  X->InitMCCodeGenInfo(RM, CM, OL);
  return X;
}

static MCInstPrinter *createVideoCore4MCInstPrinter(const Target &T,
                                                  unsigned SyntaxVariant,
                                                  const MCAsmInfo &MAI,
                                                  const MCInstrInfo &MII,
                                                  const MCRegisterInfo &MRI,
                                                  const MCSubtargetInfo &STI) {
  if (SyntaxVariant == 0)
    return new VideoCore4InstPrinter(MAI, MII, MRI);
  return 0;
}

extern "C" void LLVMInitializeVideoCore4TargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfo<VideoCore4MCAsmInfo> X(TheVideoCore4Target);

  // Register the MC codegen info.
  TargetRegistry::RegisterMCCodeGenInfo(TheVideoCore4Target,
                                        createVideoCore4MCCodeGenInfo);

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(TheVideoCore4Target, createVideoCore4MCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(TheVideoCore4Target,
                                    createVideoCore4MCRegisterInfo);

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(TheVideoCore4Target,
                                          createVideoCore4MCSubtargetInfo);

  // Register the MCInstPrinter.
  TargetRegistry::RegisterMCInstPrinter(TheVideoCore4Target,
                                        createVideoCore4MCInstPrinter);
}
