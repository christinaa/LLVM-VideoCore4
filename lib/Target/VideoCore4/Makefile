##===- lib/Target/VideoCore4/Makefile ------------------------*- Makefile -*-===##
# 
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source 
# License. See LICENSE.TXT for details.
# 
##===----------------------------------------------------------------------===##

LEVEL = ../../..
LIBRARYNAME = LLVMVideoCore4CodeGen
TARGET = VideoCore4

# Make sure that tblgen is run, first thing.
BUILT_SOURCES = VideoCore4GenRegisterInfo.inc VideoCore4GenInstrInfo.inc \
		VideoCore4GenAsmWriter.inc \
		VideoCore4GenDAGISel.inc VideoCore4GenCallingConv.inc \
		VideoCore4GenSubtargetInfo.inc

DIRS = InstPrinter TargetInfo MCTargetDesc

include $(LEVEL)/Makefile.common

