
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
# 
# toolchain_linux_riscv.cmake for Simple-XX/SimpleKernel.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR RISCV)

# GCC
find_program(RISCV riscv64-linux-gnu-g++)
if (NOT RISCV)
    message(FATAL_ERROR "riscv64-linux-gnu-gcc not found.\n"
            "Run `sudo apt-get install -y gcc-riscv64-linux-gnu g++-riscv64-linux-gnu` to install the toolchain. Then add the bin path to you PATH.")
else ()
    message(STATUS "Found riscv64-linux-gnu-gcc ${RISCV}.")
endif ()

set(TOOLCHAIN_PREFIX riscv64-linux-gnu-)
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
