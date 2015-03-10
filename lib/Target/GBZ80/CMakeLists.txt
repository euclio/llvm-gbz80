set(LLVM_TARGET_DEFINITIONS GBZ80.td)

tablegen(LLVM GBZ80GenCallingConv.inc -gen-callingconv)
tablegen(LLVM GBZ80GenDAGISel.inc -gen-dag-isel)
tablegen(LLVM GBZ80GenInstrInfo.inc -gen-instr-info)
tablegen(LLVM GBZ80GenRegisterInfo.inc -gen-register-info)
add_public_tablegen_target(GBZ80CommonTableGen)

add_llvm_target(GBZ80
    GBZ80FrameLowering.cpp
    GBZ80ISelDAGToDAG.cpp
    GBZ80ISelLowering.cpp
    GBZ80InstrInfo.cpp
    GBZ80MachineFunctionInfo.cpp
    GBZ80RegisterInfo.cpp
    GBZ80SelectionDAGInfo.cpp
    GBZ80TargetMachine.cpp
)

add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
