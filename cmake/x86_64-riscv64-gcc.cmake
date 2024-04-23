
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
# 
# x86_64-riscv64-gcc.cmake for Simple-XX/SimpleKernel.

if (NOT UNIX)
    message(FATAL_ERROR "Only support Linux.")
endif ()

if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "riscv64")
    # GCC
    find_program(Compiler_gcc g++)
    if (NOT Compiler_gcc)
        message(FATAL_ERROR "g++ not found.\n"
                "Run `sudo apt-get install -y gcc g++` to install.")
    else ()
        message(STATUS "Found g++ ${Compiler_gcc}")
    endif ()
elseif (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64")
    find_program(Compiler_gcc riscv64-linux-gnu-g++)
    if (NOT Compiler_gcc)
        message(FATAL_ERROR "riscv64-linux-gnu-g++ not found.\n"
                "Run `sudo apt install -y gcc-riscv64-linux-gnu g++-riscv64-linux-gnu` to install.")
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
elseif (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "aarch64")
    find_program(Compiler_gcc_cr riscv64-linux-gnu-g++)
    if (NOT Compiler_gcc_cr)
        message(FATAL_ERROR "riscv64-linux-gnu-g++ not found.\n"
                "Run `sudo apt install -y g++-riscv64-linux-gnu` to install.")
    else ()
        message(STATUS "Found riscv64-linux-gnu-g++  ${Compiler_gcc_cr}")
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
else ()
    message(FATAL_ERROR "NOT support ${CMAKE_HOST_SYSTEM_PROCESSOR}")
endif ()
