//===-- VideoCore4ISelLowering.cpp - VideoCore4 DAG Lowering Implementation  --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the VideoCore4TargetLowering class.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "vc4-lower"

#include "VideoCore4ISelLowering.h"
#include "VideoCore4.h"
#include "VideoCore4MachineFunctionInfo.h"
#include "VideoCore4TargetMachine.h"
#include "VideoCore4Subtarget.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/GlobalAlias.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

VideoCore4TargetLowering::VideoCore4TargetLowering(VideoCore4TargetMachine &tm) :
	TargetLowering(tm, new TargetLoweringObjectFileELF()),
	Subtarget(*tm.getSubtargetImpl()) {

	TD = getDataLayout();

	// Set up the register classes.
	addRegisterClass(MVT::i32, &VideoCore4::GR32RegClass);
	addRegisterClass(MVT::f32, &VideoCore4::GR32RegClass);
	addRegisterClass(MVT::i32, &VideoCore4::FR32RegClass);
	addRegisterClass(MVT::f32, &VideoCore4::FR32RegClass);
	//addRegisterClass(MVT::i32, &VideoCore4::IR32RegClass);

	// Compute derived properties from the register classes
	computeRegisterProperties();

	setOperationAction(ISD::GlobalAddress, MVT::i32, Custom);
	setOperationAction(ISD::BR_CC,         MVT::i32, Expand);
	setOperationAction(ISD::BR_CC,         MVT::f32, Expand);
	setOperationAction(ISD::SELECT_CC,     MVT::i32, Expand);

	setOperationAction(ISD::ADDC, MVT::i32, Expand);
	setOperationAction(ISD::ADDE, MVT::i32, Expand);
	setOperationAction(ISD::SUBC, MVT::i32, Expand);
	setOperationAction(ISD::SUBE, MVT::i32, Expand);
  
	setOperationAction(ISD::ConstantFP,    MVT::f32, Legal);

	setOperationAction(ISD::UDIV,          MVT::i32, Legal);
	setOperationAction(ISD::SDIV,          MVT::i32, Legal);

	//setOperationAction(ISD::MULHU,         MVT::i32, Expand);
	setOperationAction(ISD::UREM,          MVT::i32, Expand);
	setOperationAction(ISD::UDIVREM,       MVT::i32, Expand);

	setOperationAction(ISD::BR_JT, MVT::Other, Custom);

	// Varargs
	setOperationAction(ISD::VAEND, MVT::Other, Expand);
	setOperationAction(ISD::VACOPY, MVT::Other, Expand);
	setOperationAction(ISD::VAARG, MVT::Other, Custom);
	setOperationAction(ISD::VASTART, MVT::Other, Custom);

	setBooleanContents(ZeroOrOneBooleanContent);

	// Sign extend on some loads.
	setLoadExtAction(ISD::SEXTLOAD, MVT::i1, Promote);
	setLoadExtAction(ISD::SEXTLOAD, MVT::i8, Expand);

	setOperationAction(ISD::STACKSAVE, MVT::Other, Expand);
	setOperationAction(ISD::STACKRESTORE, MVT::Other, Expand);

	setMinFunctionAlignment(1);
	setPrefFunctionAlignment(1);
}

SDValue VideoCore4TargetLowering::
LowerVAARG(SDValue Op, SelectionDAG &DAG) const
{
	// Whist llvm does not support aggregate varargs we can ignore
	// the possibility of the ValueType being an implicit byVal vararg.
	SDNode *Node = Op.getNode();
	EVT VT = Node->getValueType(0); // not an aggregate
	SDValue InChain = Node->getOperand(0);
	SDValue VAListPtr = Node->getOperand(1);
	EVT PtrVT = VAListPtr.getValueType();
	const Value *SV = cast<SrcValueSDNode>(Node->getOperand(2))->getValue();
	SDLoc dl(Node);
	SDValue VAList = DAG.getLoad(PtrVT, dl, InChain,
															 VAListPtr, MachinePointerInfo(SV),
															 false, false, false, 0);
	// Increment the pointer, VAList, to the next vararg
	SDValue nextPtr = DAG.getNode(ISD::ADD, dl, PtrVT, VAList,
																DAG.getIntPtrConstant(VT.getSizeInBits() / 8));
	// Store the incremented VAList to the legalized pointer
	InChain = DAG.getStore(VAList.getValue(1), dl, nextPtr, VAListPtr,
												 MachinePointerInfo(SV), false, false, 0);
	// Load the actual argument out of the pointer VAList
	return DAG.getLoad(VT, dl, InChain, VAList, MachinePointerInfo(),
										 false, false, false, 0);
}

SDValue VideoCore4TargetLowering::
LowerVASTART(SDValue Op, SelectionDAG &DAG) const
{
	SDLoc dl(Op);
	// vastart stores the address of the VarArgsFrameIndex slot into the
	// memory location argument

	MachineFunction &MF = DAG.getMachineFunction();
	VideoCore4MachineFunctionInfo *XFI = MF.getInfo<VideoCore4MachineFunctionInfo>();

	//errs() << "VASTART at " << XFI->VarArgsFrameIndex << "\n";

	SDValue Addr = DAG.getFrameIndex(XFI->VarArgsFrameIndex, MVT::i32);
	return DAG.getStore(Op.getOperand(0), dl, Addr, Op.getOperand(1),
											MachinePointerInfo(), false, false, 0);
}

SDValue VideoCore4TargetLowering::
LowerBR_JT(SDValue Op, SelectionDAG &DAG) const
{
	MachineFunction &MF = DAG.getMachineFunction();
	const MachineJumpTableInfo *MJTI = MF.getJumpTableInfo();
	MachineRegisterInfo &RegInfo = MF.getRegInfo();

	SDValue Chain = Op.getOperand(0);
	SDValue Table = Op.getOperand(1);
	SDValue Index = Op.getOperand(2);
	SDLoc dl(Op);

	JumpTableSDNode *JT = cast<JumpTableSDNode>(Table);
	unsigned JTI = JT->getIndex();
	SDValue TargetJT = DAG.getTargetJumpTable(JT->getIndex(), MVT::i32);
	return DAG.getNode(VideoCore4ISD::BR_JT, dl, MVT::Other, Chain, TargetJT, Index);
}

SDValue VideoCore4TargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
	EVT PtrVT = getPointerTy();
	SDLoc dl(Op);
	

	switch (Op.getOpcode()) {
		case ISD::VAARG:        return LowerVAARG(Op, DAG);
		case ISD::VASTART:      return LowerVASTART(Op, DAG);
		case ISD::BR_JT:        return LowerBR_JT(Op, DAG);
		case ISD::GlobalAddress: {
			const GlobalValue* GV = cast<GlobalAddressSDNode>(Op)->getGlobal();
			return DAG.getNode(VideoCore4ISD::GLOBAL, dl, PtrVT, DAG.getTargetGlobalAddress(GV, dl, PtrVT));
		}
		
		case ISD::FRAMEADDR:
			assert(false);
		default:
			llvm_unreachable("unimplemented operand");
	}
}

//===----------------------------------------------------------------------===//
//                      Calling Convention Implementation
//===----------------------------------------------------------------------===//

#include "VideoCore4GenCallingConv.inc"
#include "VideoCore4GenRegisterInfo.inc"

namespace {
	struct ArgDataPair { SDValue SDV; ISD::ArgFlagsTy Flags; };
}

SDValue
VideoCore4TargetLowering::LowerFormalArguments(SDValue Chain,
																					 CallingConv::ID CallConv,
																					 bool isVarArg,
																					 const SmallVectorImpl<ISD::InputArg>
																						 &Ins,
																					 SDLoc dl,
																					 SelectionDAG &DAG,
																					 SmallVectorImpl<SDValue> &InVals)
																						 const {

	switch (CallConv) {
	default:
		llvm_unreachable("Unsupported calling convention");
	case CallingConv::C:
	case CallingConv::Fast:
		return LowerCCCArguments(Chain, CallConv, isVarArg, Ins, dl, DAG, InVals);
	}
}

SDValue
VideoCore4TargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
																SmallVectorImpl<SDValue> &InVals) const {
	SelectionDAG &DAG                     = CLI.DAG;
	SDLoc &dl                             = CLI.DL;
	SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
	SmallVectorImpl<SDValue> &OutVals     = CLI.OutVals;
	SmallVectorImpl<ISD::InputArg> &Ins   = CLI.Ins;
	SDValue Chain                         = CLI.Chain;
	SDValue Callee                        = CLI.Callee;
	bool &isTailCall                      = CLI.IsTailCall;
	CallingConv::ID CallConv              = CLI.CallConv;
	bool isVarArg                         = CLI.IsVarArg;

	// VideoCore4 target does not yet support tail call optimization.
	isTailCall = false;

	switch (CallConv) {
	default:
		llvm_unreachable("Unsupported calling convention");
	case CallingConv::Fast:
	case CallingConv::C:
		return LowerCCCCallTo(Chain, Callee, CallConv, isVarArg, isTailCall,
													Outs, OutVals, Ins, dl, DAG, InVals);
	}
}

/// LowerCCCArguments - transform physical registers into virtual registers and
/// generate load operations for arguments places on the stack.
SDValue
VideoCore4TargetLowering::LowerCCCArguments(SDValue Chain,
																					CallingConv::ID CallConv,
																					bool isVarArg,
																					const SmallVectorImpl<ISD::InputArg>
																					&Ins,
																					SDLoc dl,
																					SelectionDAG &DAG,
																					SmallVectorImpl<SDValue> &InVals)
																					const {
	MachineFunction &MF = DAG.getMachineFunction();
	MachineFrameInfo *MFI = MF.getFrameInfo();
	MachineRegisterInfo &RegInfo = MF.getRegInfo();

	// Assign locations to all of the incoming arguments.
	SmallVector<CCValAssign, 16> ArgLocs;
	CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
								 getTargetMachine(), ArgLocs, *DAG.getContext());
	CCInfo.AnalyzeFormalArguments(Ins, CC_VideoCore4);

	SmallVector<SDValue, 5> CFRegNode;
	SmallVector<ArgDataPair, 5> ArgData;
	SmallVector<SDValue, 5> MemOps;

	unsigned StackSlotSize = 4;
	unsigned LRSaveSize = 0;

	for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {

		CCValAssign &VA = ArgLocs[i];
		SDValue ArgIn;

		if (VA.isRegLoc()) {
			// Arguments passed in registers
			EVT RegVT = VA.getLocVT();
			switch (RegVT.getSimpleVT().SimpleTy) {
			default:
				{
#ifndef NDEBUG
					errs() << "LowerFormalArguments Unhandled argument type: "
								 << RegVT.getSimpleVT().SimpleTy << "\n";
#endif
					llvm_unreachable(nullptr);
				}
			case MVT::i32:
				unsigned VReg = RegInfo.createVirtualRegister(&VideoCore4::GR32RegClass);
				RegInfo.addLiveIn(VA.getLocReg(), VReg);
				ArgIn = DAG.getCopyFromReg(Chain, dl, VReg, RegVT);
				CFRegNode.push_back(ArgIn.getValue(ArgIn->getNumValues() - 1));
			}
		} else {
			// sanity check
			assert(VA.isMemLoc());
			// Load the argument to a virtual register
			unsigned ObjSize = VA.getLocVT().getSizeInBits()/8;
			if (ObjSize > StackSlotSize) {
				errs() << "LowerFormalArguments Unhandled argument type: "
							 << EVT(VA.getLocVT()).getEVTString()
							 << "\n";
			}
			// Create the frame index object for this incoming parameter...
			int FI = MFI->CreateFixedObject(ObjSize,
																			LRSaveSize + VA.getLocMemOffset(),
																			true);

			// Create the SelectionDAG nodes corresponding to a load
			//from this parameter
			SDValue FIN = DAG.getFrameIndex(FI, MVT::i32);
			ArgIn = DAG.getLoad(VA.getLocVT(), dl, Chain, FIN,
													MachinePointerInfo::getFixedStack(FI),
													false, false, false, 0);
		}
		const ArgDataPair ADP = { ArgIn, Ins[i].Flags };
		ArgData.push_back(ADP);
	}

	// 1b. CopyFromReg vararg registers.
	if (isVarArg) {
		// Argument registers
		static const MCPhysReg ArgRegs[] = {
			VideoCore4::R0, VideoCore4::R1, VideoCore4::R2, VideoCore4::R3,
			VideoCore4::R4, VideoCore4::R5
		};
		VideoCore4MachineFunctionInfo *XFI = MF.getInfo<VideoCore4MachineFunctionInfo>();
		unsigned FirstVAReg = CCInfo.getFirstUnallocated(ArgRegs, array_lengthof(ArgRegs));
		if (FirstVAReg < array_lengthof(ArgRegs)) {
			int offset = 0;
			// Save remaining registers, storing higher register numbers at a higher
			// address
			for (int i = array_lengthof(ArgRegs) - 1; i >= (int)FirstVAReg; --i) {
				// Create a stack slot
				int FI = MFI->CreateFixedObject(4, offset, true);
				if (i == (int)FirstVAReg) {
					 XFI->VarArgsFrameIndex = (FI);
				}
				offset -= StackSlotSize;
				SDValue FIN = DAG.getFrameIndex(FI, MVT::i32);
				// Move argument from phys reg -> virt reg
				unsigned VReg = RegInfo.createVirtualRegister(&VideoCore4::GR32RegClass);
				RegInfo.addLiveIn(ArgRegs[i], VReg);
				SDValue Val = DAG.getCopyFromReg(Chain, dl, VReg, MVT::i32);
				CFRegNode.push_back(Val.getValue(Val->getNumValues() - 1));
				// Move argument from virt reg -> stack
				SDValue Store = DAG.getStore(Val.getValue(1), dl, Val, FIN,
																		 MachinePointerInfo(), false, false, 0);
				MemOps.push_back(Store);
			}
		} else {
			// This will point to the next argument passed via stack.
			XFI->VarArgsFrameIndex = MFI->CreateFixedObject(4, LRSaveSize + CCInfo.getNextStackOffset(), true);
		}
	}

	// 2. chain CopyFromReg nodes into a TokenFactor.
	if (!CFRegNode.empty())
		Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, CFRegNode);

	// 3. Memcpy 'byVal' args & push final InVals.
	// Aggregates passed "byVal" need to be copied by the callee.
	// The callee will use a pointer to this copy, rather than the original
	// pointer.
	for (SmallVectorImpl<ArgDataPair>::const_iterator ArgDI = ArgData.begin(),
																										ArgDE = ArgData.end();
			 ArgDI != ArgDE; ++ArgDI) {
		if (ArgDI->Flags.isByVal() && ArgDI->Flags.getByValSize()) {
			unsigned Size = ArgDI->Flags.getByValSize();
			unsigned Align = std::max(StackSlotSize, ArgDI->Flags.getByValAlign());
			// Create a new object on the stack and copy the pointee into it.
			int FI = MFI->CreateStackObject(Size, Align, false);
			SDValue FIN = DAG.getFrameIndex(FI, MVT::i32);
			InVals.push_back(FIN);
			MemOps.push_back(DAG.getMemcpy(Chain, dl, FIN, ArgDI->SDV,
																		 DAG.getConstant(Size, MVT::i32),
																		 Align, false, false,
																		 MachinePointerInfo(),
																		 MachinePointerInfo()));
		} else {
			InVals.push_back(ArgDI->SDV);
		}
	}

	// 4, chain mem ops nodes into a TokenFactor.
	if (!MemOps.empty()) {
		MemOps.push_back(Chain);
		Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, MemOps);
	}

	return Chain;
}

SDValue
VideoCore4TargetLowering::LowerReturn(SDValue Chain,
																		CallingConv::ID CallConv, bool isVarArg,
																		const SmallVectorImpl<ISD::OutputArg> &Outs,
																		const SmallVectorImpl<SDValue> &OutVals,
																		SDLoc dl, SelectionDAG &DAG) const {

	// CCValAssign - represent the assignment of the return value to a location
	SmallVector<CCValAssign, 16> RVLocs;

	// CCState - Info about the registers and stack slot.
	CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
								 getTargetMachine(), RVLocs, *DAG.getContext());

	// Analize return values.
	CCInfo.AnalyzeReturn(Outs, RetCC_VideoCore4);

	SDValue Flag;
	SmallVector<SDValue, 4> RetOps(1, Chain);

	// Copy the result values into the output registers.
	for (unsigned i = 0; i != RVLocs.size(); ++i) {
		CCValAssign &VA = RVLocs[i];
		assert(VA.isRegLoc() && "Can only return in registers!");

		Chain = DAG.getCopyToReg(Chain, dl, VA.getLocReg(),
														 OutVals[i], Flag);

		// Guarantee that all emitted copies are stuck together,
		// avoiding something bad.
		Flag = Chain.getValue(1);
		RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
	}

	RetOps[0] = Chain; // Update chain.

	// Add the flag if we have it.
	if (Flag.getNode())
		RetOps.push_back(Flag);

	return DAG.getNode(VideoCore4ISD::RET_FLAG, dl, MVT::Other,
			ArrayRef<SDValue>(&RetOps[0], RetOps.size()));
}


/// LowerCallResult - Lower the result values of a call into the
/// appropriate copies out of appropriate physical registers.
///
static SDValue
__LowerCallResult(SDValue Chain, SDValue InFlag,
                const SmallVectorImpl<CCValAssign> &RVLocs,
                SDLoc dl, SelectionDAG &DAG,
                SmallVectorImpl<SDValue> &InVals)
{
  SmallVector<std::pair<int, unsigned>, 5> ResultMemLocs;

  // Copy results out of physical registers.
  for (unsigned i = 0, e = RVLocs.size(); i != e; ++i) {
    const CCValAssign &VA = RVLocs[i];
    if (VA.isRegLoc()) {
      Chain = DAG.getCopyFromReg(Chain, dl, VA.getLocReg(), VA.getValVT(),
                                 InFlag).getValue(1);
      InFlag = Chain.getValue(2);
      InVals.push_back(Chain.getValue(0));
    } else {
    	llvm_unreachable("stack based return not implemented yet");

      assert(VA.isMemLoc());
      ResultMemLocs.push_back(std::make_pair(VA.getLocMemOffset(),
                                             InVals.size()));
      // Reserve space for this result.
      InVals.push_back(SDValue());
    }
  }

  SmallVector<SDValue, 5> MemOpChains;

#if 0
  // Copy results out of memory.
  
  for (unsigned i = 0, e = ResultMemLocs.size(); i != e; ++i) {
    int offset = ResultMemLocs[i].first;
    unsigned index = ResultMemLocs[i].second;
    SDVTList VTs = DAG.getVTList(MVT::i32, MVT::Other);
    SDValue Ops[] = { Chain, DAG.getConstant(offset / 4, MVT::i32) };
    SDValue load = DAG.getNode(XCoreISD::LDWSP, dl, VTs, Ops);
    InVals[index] = load;
    MemOpChains.push_back(load.getValue(1));
  }
#endif

  // Transform all loads nodes into one single node because
  // all load nodes are independent of each other.
  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, MemOpChains);

  return Chain;
}

/// LowerCCCCallTo - functions arguments are copied from virtual regs to
/// (physical regs)/(stack frame), CALLSEQ_START and CALLSEQ_END are emitted.
/// TODO: sret.
SDValue
VideoCore4TargetLowering::LowerCCCCallTo(SDValue Chain, SDValue Callee,
																			 CallingConv::ID CallConv, bool isVarArg,
																			 bool isTailCall,
																			 const SmallVectorImpl<ISD::OutputArg>
																			 &Outs,
																			 const SmallVectorImpl<SDValue> &OutVals,
																			 const SmallVectorImpl<ISD::InputArg> &Ins,
																			 SDLoc dl, SelectionDAG &DAG,
																			 SmallVectorImpl<SDValue> &InVals) const
{
  // Analyze operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                 getTargetMachine(), ArgLocs, *DAG.getContext());

  // The ABI dictates there should be one stack slot available to the callee
  // on function entry (for saving lr).
  CCInfo.AllocateStack(4, 4);

  CCInfo.AnalyzeCallOperands(Outs, CC_VideoCore4);

  SDValue StackPtr;

  SmallVector<CCValAssign, 16> RVLocs;
  // Analyze return values to determine the number of bytes of stack required.
  CCState RetCCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                    getTargetMachine(), RVLocs, *DAG.getContext());
  RetCCInfo.AllocateStack(CCInfo.getNextStackOffset(), 4);
  RetCCInfo.AnalyzeCallResult(Ins, RetCC_VideoCore4);

  // Get a count of how many bytes are to be pushed on the stack.
  unsigned NumBytes = RetCCInfo.getNextStackOffset();

  Chain = DAG.getCALLSEQ_START(Chain,DAG.getConstant(NumBytes,
                                 getPointerTy(), true), dl);

  SmallVector<std::pair<unsigned, SDValue>, 5> RegsToPass;
  SmallVector<SDValue, 12> MemOpChains;

  // Walk the register/memloc assignments, inserting copies/loads.
  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    SDValue Arg = OutVals[i];

    // Promote the value if needed.
    switch (VA.getLocInfo()) {
      default: llvm_unreachable("Unknown loc info!");
      case CCValAssign::Full: break;
      case CCValAssign::SExt:
        Arg = DAG.getNode(ISD::SIGN_EXTEND, dl, VA.getLocVT(), Arg);
        break;
      case CCValAssign::ZExt:
        Arg = DAG.getNode(ISD::ZERO_EXTEND, dl, VA.getLocVT(), Arg);
        break;
      case CCValAssign::AExt:
        Arg = DAG.getNode(ISD::ANY_EXTEND, dl, VA.getLocVT(), Arg);
        break;
    }

    // Arguments that can be passed on register must be kept at
    // RegsToPass vector
    if (VA.isRegLoc()) {
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
    } else {
      assert(VA.isMemLoc());

			if (StackPtr.getNode() == 0)
				StackPtr = DAG.getCopyFromReg(Chain, dl, VideoCore4::SP, getPointerTy());

			SDValue PtrOff = DAG.getNode(ISD::ADD, dl, getPointerTy(),
																	 StackPtr,
																	 DAG.getIntPtrConstant(VA.getLocMemOffset()));

			MemOpChains.push_back(DAG.getStore(Chain, dl, Arg, PtrOff,
																				 MachinePointerInfo(),false, false, 0));
    }
  }

  // Transform all store nodes into one single node because
  // all store nodes are independent of each other.
  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, MemOpChains);

  // Build a sequence of copy-to-reg nodes chained together with token
  // chain and flag operands which copy the outgoing args into registers.
  // The InFlag in necessary since all emitted instructions must be
  // stuck together.
  SDValue InFlag;
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i) {
    Chain = DAG.getCopyToReg(Chain, dl, RegsToPass[i].first,
                             RegsToPass[i].second, InFlag);
    InFlag = Chain.getValue(1);
  }

  // If the callee is a GlobalAddress node (quite common, every direct call is)
  // turn it into a TargetGlobalAddress node so that legalize doesn't hack it.
  // Likewise ExternalSymbol -> TargetExternalSymbol.
  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee))
    Callee = DAG.getTargetGlobalAddress(G->getGlobal(), dl, MVT::i32);
  else if (ExternalSymbolSDNode *E = dyn_cast<ExternalSymbolSDNode>(Callee))
    Callee = DAG.getTargetExternalSymbol(E->getSymbol(), MVT::i32);

  // XCoreBranchLink = #chain, #target_address, #opt_in_flags...
  //             = Chain, Callee, Reg#1, Reg#2, ...
  //
  // Returns a chain & a flag for retval copy to use.
  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  SmallVector<SDValue, 8> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);

  // Add argument registers to the end of the list so that they are
  // known live into the call.
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i)
    Ops.push_back(DAG.getRegister(RegsToPass[i].first,
                                  RegsToPass[i].second.getValueType()));

  if (InFlag.getNode())
    Ops.push_back(InFlag);

  Chain  = DAG.getNode(VideoCore4ISD::CALL, dl, NodeTys, ArrayRef<SDValue>(&Ops[0], Ops.size()));
  InFlag = Chain.getValue(1);

  // Create the CALLSEQ_END node.
  Chain = DAG.getCALLSEQ_END(Chain,
                             DAG.getConstant(NumBytes, getPointerTy(), true),
                             DAG.getConstant(0, getPointerTy(), true),
                             InFlag, dl);
  InFlag = Chain.getValue(1);

  // Handle result values, copying them out of physregs into vregs that we
  // return.
  return __LowerCallResult(Chain, InFlag, RVLocs, dl, DAG, InVals);
}

const char *VideoCore4TargetLowering::getTargetNodeName(unsigned Opcode) const {
	switch (Opcode) {
	default: return NULL;
	case VideoCore4ISD::RET_FLAG:           return "VideoCore4ISD::RET_FLAG";
	case VideoCore4ISD::CALL:               return "VideoCore4ISD::CALL";
	case VideoCore4ISD::GLOBAL:             return "VideoCore4ISD::GLOBAL";
	case VideoCore4ISD::BR_JT:              return "VideoCore4ISD::BR_JT";
	}
}
