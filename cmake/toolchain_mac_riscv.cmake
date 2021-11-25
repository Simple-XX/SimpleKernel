
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
# 
# toolchain_mac_riscv.cmake for Simple-XX/SimpleKernel.

set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR RISCV)

find_program(BREW brew)
if (NOT BREW)
    message(FATAL_ERROR "brew not found.\n"
            "Run `/bin/bash -c \"$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)\"` to install linuxbrew")
else ()
    message(STATUS "Found brew ${BREW}")
endif ()

# GCC
find_program(RISCV riscv64-unknown-elf-gcc)
if (NOT RISCV)
    message(FATAL_ERROR "riscv64-unknown-elf-gcc not found.\n"
            "Run `brew tap riscv/riscv` and `brew install riscv-tools` to install the toolchain. Then add the bin path to you PATH.")
else ()
    message(STATUS "Found riscv64-unknown-elf-gcc ${RISCV}.")
endif ()

set(TOOLCHAIN_PREFIX riscv64-unknown-elf-)
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)

# qemu
find_program(QEMU qemu-system-riscv64)
if (NOT QEMU)
    message(FATAL_ERROR "qemu not found.\n"
            "Please install qemu first.")
else ()
    message(STATUS "Found qemu ${QEMU}")
endif ()

# opensbi
# cd tools
# git clone https://github.com/riscv/opensbi.git
# cd opensbi
# export CROSS_COMPILE=riscv64-unknown-elf-
# make PLATFORM=generic clean
# make PLATFORM=generic FW_JUMP_ADDR=0x80200000
