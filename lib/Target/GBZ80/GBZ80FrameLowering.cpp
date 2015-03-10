//===-- GBZ80FrameLowering.cpp - GBZ80 Frame Information ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the GBZ80 implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "GBZ80FrameLowering.h"
#include "GBZ80.h"
#include "GBZ80InstrInfo.h"
#include "GBZ80MachineFunctionInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
using namespace llvm;

GBZ80FrameLowering::GBZ80FrameLowering(const GBZ80TargetMachine &tm)
  : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, 1, -2), TM(tm)
{}

bool GBZ80FrameLowering::hasFP(const MachineFunction &MF) const
{
  const MachineFrameInfo *MFI = MF.getFrameInfo();
  return (MFI->getMaxCallFrameSize() > 0 || (MFI->getNumObjects() > 0));
}

void GBZ80FrameLowering::emitPrologue(MachineFunction &MF) const
{
  MachineBasicBlock &MBB = MF.front();  // Prolog goes into entry BB
  MachineBasicBlock::iterator MBBI = MBB.begin();
  MachineFrameInfo *MFI = MF.getFrameInfo();
  GBZ80MachineFunctionInfo *GBZ80FI = MF.getInfo<GBZ80MachineFunctionInfo>();
  const GBZ80InstrInfo &TII = 
    *static_cast<const GBZ80InstrInfo*>(MF.getSubtarget().getInstrInfo());
  DebugLoc dl = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();

  uint64_t StackSize     = MFI->getStackSize();
  uint64_t CallFrameSize = MFI->getMaxCallFrameSize();
  uint64_t FrameSize     = GBZ80FI->getCalleeSavedFrameSize();

  uint64_t NumBytes = StackSize + CallFrameSize - FrameSize;

  // Skip the callee-saved push instructions.
  while (MBBI != MBB.end() && (MBBI->getOpcode() == GBZ80::PUSH16r))
    MBBI++;

  if (NumBytes || GBZ80FI->isNeedFP())
  {
    unsigned FP = TII.getRegisterInfo().getFrameRegister(MF);

    BuildMI(MBB, MBBI, dl, TII.get(GBZ80::LD16ri), FP)
      .addImm(-NumBytes);
    BuildMI(MBB, MBBI, dl, TII.get(GBZ80::ADD16rSP), FP);
    BuildMI(MBB, MBBI, dl, TII.get(GBZ80::LD16SPr))
      .addReg(FP);
  }
}

void GBZ80FrameLowering::emitEpilogue(MachineFunction &MF,
  MachineBasicBlock &MBB) const
{
  const MachineFrameInfo *MFI = MF.getFrameInfo();
  GBZ80MachineFunctionInfo *GBZ80FI = MF.getInfo<GBZ80MachineFunctionInfo>();
  const GBZ80InstrInfo &TII =
    *static_cast<const GBZ80InstrInfo*>(MF.getSubtarget().getInstrInfo());

  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  unsigned RetOpcode = MBBI->getOpcode();
  DebugLoc dl = MBBI->getDebugLoc();

  if (RetOpcode != GBZ80::RET)
    llvm_unreachable("Can only insert epilog into returning blocks");

  // Get the number of bytes to allocate from the FrameInfo
  uint64_t StackSize     = MFI->getStackSize();
  uint64_t CallFrameSize = MFI->getMaxCallFrameSize();
  uint64_t FrameSize     = GBZ80FI->getCalleeSavedFrameSize();

  uint64_t NumBytes = StackSize + CallFrameSize - FrameSize;

  // Skip the callee-saved pop instructions.
  while (MBBI != MBB.begin())
  {
    MachineBasicBlock::iterator I = std::prev(MBBI);
    unsigned Opc = I->getOpcode();
    if (Opc != GBZ80::POP16r && !I->isTerminator())
      break;
    MBBI--;
  }

  if (NumBytes)
  {
    unsigned FP = TII.getRegisterInfo().getFrameRegister(MF);

    BuildMI(MBB, MBBI, dl, TII.get(GBZ80::LD16ri), FP)
      .addImm(NumBytes);
    BuildMI(MBB, MBBI, dl, TII.get(GBZ80::ADD16rSP), FP);
    BuildMI(MBB, MBBI, dl, TII.get(GBZ80::LD16SPr))
      .addReg(FP, RegState::Kill);
  }
}

bool GBZ80FrameLowering::spillCalleeSavedRegisters(MachineBasicBlock &MBB,
  MachineBasicBlock::iterator MI,
  const std::vector<CalleeSavedInfo> &CSI,
  const TargetRegisterInfo *TRI) const
{
  if (CSI.empty())
    return false;

  DebugLoc dl;
  if (MI != MBB.end()) dl = MI->getDebugLoc();

  MachineFunction &MF = *MBB.getParent();
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  GBZ80MachineFunctionInfo *MFI = MF.getInfo<GBZ80MachineFunctionInfo>();
  MFI->setCalleeSavedFrameSize(CSI.size() * 2);

  for (unsigned i = CSI.size(); i != 0; i--)
  {
    unsigned Reg = CSI[i-1].getReg();

    // Add the callee-saved register as live-in. It's killed at the spill.
    MBB.addLiveIn(Reg);
    BuildMI(MBB, MI, dl, TII.get(GBZ80::PUSH16r))
      .addReg(Reg, RegState::Kill);
  }
  return true;
}

bool GBZ80FrameLowering::restoreCalleeSavedRegisters(MachineBasicBlock &MBB,
  MachineBasicBlock::iterator MI,
  const std::vector<CalleeSavedInfo> &CSI,
  const TargetRegisterInfo *TRI) const
{
  if (CSI.empty())
    return false;

  DebugLoc dl;
  if (MI != MBB.end()) dl = MI->getDebugLoc();

  MachineFunction &MF = *MBB.getParent();
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();

  for (unsigned i = 0, e = CSI.size(); i != e; i++)
    BuildMI(MBB, MI, dl, TII.get(GBZ80::POP16r), CSI[i].getReg());

  return true;
}

void GBZ80FrameLowering::processFunctionBeforeCalleeSavedScan(
  MachineFunction &MF, RegScavenger *RS) const
{
  if (hasFP(MF))
  {
    unsigned FP = MF.getSubtarget().getRegisterInfo()->getFrameRegister(MF);
    MF.getRegInfo().setPhysRegUsed(FP);
  }
}

void GBZ80FrameLowering::eliminateCallFramePseudoInstr(MachineFunction &MF,
  MachineBasicBlock &MBB, MachineBasicBlock::iterator I) const
{
  MachineInstr &MI = *I;

  switch (MI.getOpcode())
  {
  default: llvm_unreachable("Cannot handle this call frame pseudo instruction");
  case GBZ80::ADJCALLSTACKDOWN:
  case GBZ80::ADJCALLSTACKUP:
    break;
  }
  MBB.erase(I);
}
