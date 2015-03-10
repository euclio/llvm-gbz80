//===-- GBZ80InstrInfo.cpp - GBZ80 Instruction Information ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the GBZ80 implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "GBZ80InstrInfo.h"
#include "GBZ80.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetSubtargetInfo.h"

#define GET_INSTRINFO_CTOR
#include "GBZ80GenInstrInfo.inc"

using namespace llvm;

// FIXME Figure out corrent registers here and elsewhere.
// We no longer have the xmi instructions, so rxm is rr/rm, etc.
GBZ80InstrInfo::GBZ80InstrInfo(GBZ80TargetMachine &tm)
  : GBZ80GenInstrInfo(GBZ80::ADJCALLSTACKDOWN, GBZ80::ADJCALLSTACKUP),
  RI(tm, *this), TM(tm)
{}

void GBZ80InstrInfo::copyPhysReg(MachineBasicBlock &MBB,
  MachineBasicBlock::iterator I, DebugLoc DL,
  unsigned DestReg, unsigned SrcReg, bool KillSrc) const
{
  if (GBZ80::GR8RegClass.contains(DestReg, SrcReg))
  {
    // copy GR8 to GR8
    BuildMI(MBB, I, DL, get(GBZ80::LD8rr), DestReg)
      .addReg(SrcReg, getKillRegState(KillSrc));
    return;
  }
  else if (GBZ80::GR16RegClass.contains(DestReg, SrcReg))
  {
    // copy BR16 to BR16
    unsigned DestSubReg, SrcSubReg;

    DestSubReg = RI.getSubReg(DestReg, GBZ80::subreg_hi);
    SrcSubReg  = RI.getSubReg(SrcReg,  GBZ80::subreg_hi);
    BuildMI(MBB, I, DL, get(GBZ80::LD8rr), DestSubReg)
      .addReg(SrcSubReg);

    DestSubReg = RI.getSubReg(DestReg, GBZ80::subreg_lo);
    SrcSubReg  = RI.getSubReg(SrcReg,  GBZ80::subreg_lo);
    BuildMI(MBB, I, DL, get(GBZ80::LD8rr), DestSubReg)
      .addReg(SrcSubReg);

    if (KillSrc)
      BuildMI(MBB, I, DL, get(GBZ80::KILL))
        .addReg(SrcReg);
    return;
  }
  else if (GBZ80::GR16RegClass.contains(DestReg) ||
           GBZ80::GR16RegClass.contains(SrcReg))
  {
    // copy GR16 to GR16
    BuildMI(MBB, I, DL, get(GBZ80::PUSH16r))
      .addReg(SrcReg, getKillRegState(KillSrc));
    BuildMI(MBB, I, DL, get(GBZ80::POP16r), DestReg);
    return;
  }
  llvm_unreachable("Imposible reg-to-reg copy");
}

MachineInstr *GBZ80InstrInfo::commuteInstruction(MachineInstr *MI,
  bool NewMI) const
{
  switch (MI->getOpcode())
  {
  default: return TargetInstrInfo::commuteInstruction(MI, NewMI);
  case GBZ80::ADD8r:
  case GBZ80::ADD16r:
  case GBZ80::ADC8r:
  case GBZ80::ADC16r:
  case GBZ80::AND8r:
  case GBZ80::XOR8r:
  case GBZ80::OR8r:
    break;
  case GBZ80::ADD8i:
  case GBZ80::ADC8i:
  case GBZ80::AND8i:
  case GBZ80::XOR8i:
  case GBZ80::OR8i:
    return NULL;
  }
  assert(!NewMI && "Not implemented yet!");

  MachineBasicBlock &MBB = *MI->getParent();
  MachineFunction &MF = *MBB.getParent();
  unsigned reg[2], arg[] = { 0, 0 };

  MachineInstr *MILoadReg = MI->getPrevNode();
  if (MILoadReg == NULL || MILoadReg->getOpcode() != GBZ80::COPY) return NULL;

  MachineOperand &MO0 = MI->getOperand(0);
  MachineOperand &MO1 = MILoadReg->getOperand(1);
  reg[0] = MO0.getReg();
  reg[1] = MO1.getReg();

  // DEBUG(dbgs() << "COMMUTING:\n\t" << *MILoadReg << "\t" << *MI);
  // DEBUG(dbgs() << "COMMUTING OPERANDS: " << MO0 << ", " << MO1 << "\n");
  unsigned PreferArg = -1;

  for (MachineFunction::iterator MFI = MF.begin(), MFE = MF.end(); MFI != MFE; MFI++)
  {
    MachineBasicBlock::iterator MBBI = MFI->begin();
    while (MBBI != MFI->end())
    {
      if (MBBI->getOpcode() == TargetOpcode::COPY)
      {
        if (MBBI->findRegisterDefOperand(reg[0])) {
          // DEBUG(dbgs() << "DEFINE OPERAND " << MO0 << ":\n\t" << *MBBI);
          arg[0] = MBBI->getOperand(1).getReg();
          if (RI.isPhysicalRegister(arg[0])) PreferArg = 0;
        }
        if (MBBI->findRegisterDefOperand(reg[1])) {
          // DEBUG(dbgs() << "DEFINE OPERAND " << MO1 << ":\n\t" << *MBBI);
          arg[1] = MBBI->getOperand(1).getReg();
          if (RI.isPhysicalRegister(arg[0])) PreferArg = 1;
        }
        if (arg[0] && arg[1]) break;
      }
      MBBI++;
    }
    if (arg[0] && arg[1]) break;
  }

  if (arg[0] == 0 || arg[1] == 0)
  {
    // DEBUG(dbgs() << "COPY TO OPERANDS NOT FOUND\n");
    return NULL;
  }

  if (PreferArg == 0)
  {
    MO0.setReg(reg[1]);
    MO1.setReg(reg[0]);
    // DEBUG(dbgs() << "COMMUTING TO:\n\t" << *MILoadReg << "\t" << *MI);
  }
  else {
    // DEBUG(dbgs() << "COMMUTING NOT NEEDED\n");
  }
  return NULL;
}

bool GBZ80InstrInfo::AnalyzeBranch(MachineBasicBlock &MBB,
  MachineBasicBlock *&TBB, MachineBasicBlock *&FBB,
  SmallVectorImpl<MachineOperand> &Cond, bool AllowModify = false) const
{
  // Start from the bottom of the block and work up, examining the
  // terminator instructions.
  MachineBasicBlock::iterator I = MBB.end();

  while (I != MBB.begin())
  {
    I--;
    if (I->isDebugValue())
      continue;

    // Working from the bottom, when we see a non-terminator
    // instruction, we're done.
    if (!isUnpredicatedTerminator(I))
      break;

    // A terminator that isn't a branch can't easily be handled
    // by this analisys.
    if (!I->isBranch())
      return true;

    // Handle unconditional branches.
    if (I->getOpcode() == GBZ80::JP)
    {
      if (!AllowModify)
      {
        TBB = I->getOperand(0).getMBB();
        continue;
      }

      // If the block has any instructions after a JP, delete them.
      while (std::next(I) != MBB.end())
        std::next(I)->eraseFromParent();

      Cond.clear();
      FBB = 0;

      // Delete the JP if it's equivalent to a fall-through.
      if (MBB.isLayoutSuccessor(I->getOperand(0).getMBB()))
      {
        TBB = 0;
        I->eraseFromParent();
        I = MBB.end();
        continue;
      }
      // TBB is used to indicate the unconditional destination.
      TBB = I->getOperand(0).getMBB();
      continue;
    }

    // Handle conditional branches.
    assert(I->getOpcode() == GBZ80::JPCC && "Invalid conditional branch");
    GBZ80::CondCode GBZ80CC = static_cast<GBZ80::CondCode>(I->getOperand(1).getImm());
    if (GBZ80CC == GBZ80::COND_INVALID)
      return true;

    // Working from the bottom, handle the first conditional branch.
    if (Cond.empty())
    {
      FBB = TBB;
      TBB = I->getOperand(0).getMBB();
      Cond.push_back(MachineOperand::CreateImm(GBZ80CC));
      continue;
    }

    // Handle subsequent conditional branches.
    assert(0 && "Not implemented yet!");
  }
  return false;
}

unsigned GBZ80InstrInfo::RemoveBranch(MachineBasicBlock &MBB) const
{
  MachineBasicBlock::iterator I = MBB.end();
  unsigned Count = 0;

  while (I != MBB.begin())
  {
    I--;
    if (I->isDebugValue())
      continue;
    if (I->getOpcode() != GBZ80::JP &&
        I->getOpcode() != GBZ80::JPCC)
        break;
    // Remove branch.
    I->eraseFromParent();
    I = MBB.end();
    Count++;
  }
  return Count;
}

unsigned GBZ80InstrInfo::InsertBranch(MachineBasicBlock &MBB,
  MachineBasicBlock *TBB, MachineBasicBlock *FBB,
  const SmallVectorImpl<MachineOperand> &Cond,
  DebugLoc DL) const
{
  // Shouldn't be a fall through.
  assert(TBB && "InsertBranch must not be told to insert a fallthrough");
  assert((Cond.size() == 0 || Cond.size() == 1) &&
    "GBZ80 branch conditions must have one component!");

  if (Cond.empty())
  {
    // Unconditional branch?
    assert(!FBB && "Unconditional branch with multiple successors!");
    BuildMI(&MBB, DL, get(GBZ80::JP)).addMBB(TBB);
    return 1;
  }

  // Conditional branch.
  unsigned Count = 0;
  BuildMI(&MBB, DL, get(GBZ80::JPCC))
    .addMBB(TBB)
    .addImm(Cond[0].getImm());
  Count++;

  if (FBB)
  {
    // Two-way conditional branch. Insert the second branch.
    BuildMI(&MBB, DL, get(GBZ80::JP)).addMBB(FBB);
    Count++;
  }
  return Count;
}

bool GBZ80InstrInfo::ReverseBranchCondition(
  SmallVectorImpl<MachineOperand> &Cond) const
{
  assert(Cond.size() == 1 && "Invalid branch condition!");

  GBZ80::CondCode CC = static_cast<GBZ80::CondCode>(Cond[0].getImm());

  switch (CC)
  {
  default: return true;
  case GBZ80::COND_Z:
    CC = GBZ80::COND_NZ;
    break;
  case GBZ80::COND_NZ:
    CC = GBZ80::COND_Z;
    break;
  case GBZ80::COND_C:
    CC = GBZ80::COND_NC;
    break;
  case GBZ80::COND_NC:
    CC = GBZ80::COND_C;
    break;
  }

  Cond[0].setImm(CC);
  return false;
}

void GBZ80InstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
  MachineBasicBlock::iterator MI, unsigned SrcReg, bool isKill,
  int FrameIndex, const TargetRegisterClass *RC,
  const TargetRegisterInfo *TRI) const
{
  DebugLoc dl;
  if (MI != MBB.end()) dl = MI->getDebugLoc();

  if (RC == &GBZ80::GR8RegClass)
    BuildMI(MBB, MI, dl, get(GBZ80::LD8rr))
      .addFrameIndex(FrameIndex).addImm(0)
      .addReg(SrcReg, getKillRegState(isKill));
  else if (RC == &GBZ80::GR16RegClass ||
           GBZ80::GR16RegClass.contains(SrcReg)) {
    BuildMI(MBB, MI, dl, get(GBZ80::LD16mr))
      .addFrameIndex(FrameIndex).addImm(0)
      .addReg(SrcReg, getKillRegState(isKill));
  }
  else
    llvm_unreachable("Can't store this register to stack slot");
}

void GBZ80InstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
  MachineBasicBlock::iterator MI, unsigned DestReg,
  int FrameIndex, const TargetRegisterClass *RC,
  const TargetRegisterInfo *TRI) const
{
  DebugLoc dl;
  if (MI != MBB.end()) dl = MI->getDebugLoc();

  if (RC == &GBZ80::GR8RegClass)
    BuildMI(MBB, MI, dl, get(GBZ80::LD8rr), DestReg)
      .addFrameIndex(FrameIndex).addImm(0);
  else if (RC == &GBZ80::GR16RegClass ||
           GBZ80::GR16RegClass.contains(DestReg)) {
    BuildMI(MBB, MI, dl, get(GBZ80::LD16rm), DestReg)
      .addFrameIndex(FrameIndex).addImm(0);
  }
  else
    llvm_unreachable("Can't load this register from stack slot");
}

bool GBZ80InstrInfo::expandPostRAPseudo(MachineBasicBlock::iterator MI) const
{
  MachineBasicBlock &MBB = *MI->getParent();
  MachineFunction &MF = *MBB.getParent();
  const TargetRegisterInfo &RI = *MF.getSubtarget().getRegisterInfo();
  DebugLoc dl = MI->getDebugLoc();
  unsigned Opc, Reg, Imm, FPReg, Idx;

  switch (MI->getOpcode())
  {
  default:
    return false;
  }
  unsigned Lo, Hi;

  Lo = RI.getSubReg(Reg, GBZ80::subreg_lo);
  Hi = RI.getSubReg(Reg, GBZ80::subreg_hi);

  // DEBUG(dbgs() << "Pseudo: " << *MI);
  // DEBUG(dbgs() << "Replaced by:\n" <<
  //  "\t" << *MI->getPrevNode()->getPrevNode() <<
  //  "\t" << *MI->getPrevNode());

  MI->eraseFromParent();
  return true;
}
