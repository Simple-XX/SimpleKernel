
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
# 
# riscv64-x86_64.cmake for Simple-XX/SimpleKernel.

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv64)

# GCC
find_program(RISCV64 riscv64-linux-gnu-g++)
if (NOT RISCV64)
    message(FATAL_ERROR "riscv64-linux-gnu-gcc not found.\n"
            "Run `sudo apt-get install -y gcc-riscv64-linux-gnu g++-riscv64-linux-gnu` to install.")
else ()
    message(STATUS "Found riscv64-linux-gnu-gcc ${RISCV64}.")
endif ()

set(TOOLCHAIN_PREFIX   riscv64-linux-gnu-)
set(CMAKE_C_COMPILER   ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)
set(CMAKE_READELF      ${TOOLCHAIN_PREFIX}readelf)
set(CMAKE_AR           ${TOOLCHAIN_PREFIX}ar)
set(CMAKE_LINKER       ${TOOLCHAIN_PREFIX}ld)
set(CMAKE_NM           ${TOOLCHAIN_PREFIX}nm)
set(CMAKE_OBJDUMP      ${TOOLCHAIN_PREFIX}objdump)
set(CMAKE_RANLIB       ${TOOLCHAIN_PREFIX}ranlib)

# qemu
find_program(QEMU qemu-system-riscv64)
if (NOT QEMU)
    message(FATAL_ERROR "qemu not found.\n"
            "Run `sudo apt-get install -y qemu-system` to install.")
else ()
    message(STATUS "Found qemu ${QEMU}")
endif ()
