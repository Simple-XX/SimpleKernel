
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
# 
# x86_64-riscv64-gcc.cmake for Simple-XX/SimpleKernel.

# 目标为无操作系统的环境
set(CMAKE_SYSTEM_NAME Generic)
# 目标架构
set(CMAKE_SYSTEM_PROCESSOR riscv64)

# @todo mac 测试
if (APPLE)
    message(STATUS "Now is Apple systens.")
    # GCC
    find_program(Compiler_gcc riscv64-unknown-elf-g++)
    if (NOT Compiler_gcc)
        message(FATAL_ERROR "riscv64-unknown-elf-g++ not found.\n"
            "Following https://github.com/riscv-software-src/homebrew-riscv to install.")
    else ()
        message(STATUS "Found riscv64-unknown-elf-g++ ${Compiler_gcc}")
    endif ()

    set(TOOLCHAIN_PREFIX riscv64-unknown-elf-)
    set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
    set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)
    set(CMAKE_READELF ${TOOLCHAIN_PREFIX}readelf)
    set(CMAKE_AR ${TOOLCHAIN_PREFIX}ar)
    set(CMAKE_LINKER ${TOOLCHAIN_PREFIX}ld)
    set(CMAKE_NM ${TOOLCHAIN_PREFIX}nm)
    set(CMAKE_OBJDUMP ${TOOLCHAIN_PREFIX}objdump)
    set(CMAKE_RANLIB ${TOOLCHAIN_PREFIX}ranlib)
elseif (UNIX)
    message(STATUS "Now is UNIX-like OS's.")
    # GCC
    find_program(Compiler_gcc riscv64-linux-gnu-g++)
    if (NOT Compiler_gcc)
        message(FATAL_ERROR "riscv64-linux-gnu-g++ not found.\n"
            "Run `sudo apt install -y gcc-riscv64-linux-gnu g++-riscv64-linux-gnu` to install.")
    else ()
        message(STATUS "Found riscv64-linux-gnu-g++ ${Compiler_gcc}")
    endif ()

    set(TOOLCHAIN_PREFIX riscv64-linux-gnu-)
    set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
    set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)
    set(CMAKE_READELF ${TOOLCHAIN_PREFIX}readelf)
    set(CMAKE_AR ${TOOLCHAIN_PREFIX}ar)
    set(CMAKE_LINKER ${TOOLCHAIN_PREFIX}ld)
    set(CMAKE_NM ${TOOLCHAIN_PREFIX}nm)
    set(CMAKE_OBJDUMP ${TOOLCHAIN_PREFIX}objdump)
    set(CMAKE_RANLIB ${TOOLCHAIN_PREFIX}ranlib)
endif ()
