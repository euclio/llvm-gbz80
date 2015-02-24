set(LLVM_TARGET_DEFINITIONS GBZ80.td)

tablegen(LLVM GBZ80RegisterInfo.inc -gen-register-info)

add_public_tablegen_target(GBZ80CommonTableGen)

add_llvm_target(GBZ80
    GBZ80TargetMachine.cpp
    GBZ80RegisterInfo.cpp
)

add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
