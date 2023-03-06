
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
            "Run `sudo apt-get install -y gcc-riscv64-linux-gnu g++-riscv64-linux-gnu` to install the toolchain.")
else ()
    message(STATUS "Found riscv64-linux-gnu-gcc ${RISCV}.")
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
            "Run `sudo apt install -y qemu-system` to install qemu.")
else ()
    message(STATUS "Found qemu ${QEMU}")
endif ()
