
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
    # GCC
    find_program(Compiler_gcc g++)
    if (NOT Compiler_gcc)
        message(FATAL_ERROR "g++ not found.\n"
                "Run `sudo apt-get install -y gcc g++` to install.")
    else ()
        message(STATUS "Found g++ ${Compiler_gcc}")
    endif ()
endif ()
