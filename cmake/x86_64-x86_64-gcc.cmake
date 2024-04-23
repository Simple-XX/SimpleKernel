
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
# 
# x86_64-x86_64-gcc.cmake for Simple-XX/SimpleKernel.

# 目标为无操作系统的环境
set(CMAKE_SYSTEM_NAME Generic)
# 目标架构
set(CMAKE_SYSTEM_PROCESSOR x86_64)

if (APPLE)
    message(FATAL_ERROR "NOT support OSX, Please use linux")
elseif (UNIX)
    message(STATUS "Now is UNIX-like OS's.")

    if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64")
        # GCC
        find_program(Compiler_gcc g++)
        if (NOT Compiler_gcc)
            message(FATAL_ERROR "g++ not found.\n"
                "Run `sudo apt-get install -y gcc g++` to install.")
        else ()
            message(STATUS "Found g++ ${Compiler_gcc}")
        endif ()
    elseif (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "aarch64")
        find_program(Compiler_gcc_cr x86_64-linux-gnu-g++)
        if (NOT Compiler_gcc_cr)
            message(FATAL_ERROR "aarch64-linux-gnu-g++ not found.\n"
                    "Run `sudo apt install -y g++-multilib-x86-64-linux-gnu` to install.")
        else ()
            message(STATUS "Found x86_64-linux-gnu-g++  ${Compiler_gcc_cr}")
        endif ()

        set(TOOLCHAIN_PREFIX x86_64-linux-gnu-)
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
endif ()
