
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
# 
# x86_64-x86_64-clang.cmake for Simple-XX/SimpleKernel.

# 目标为无操作系统的环境
set(CMAKE_SYSTEM_NAME Generic)
# 目标架构
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# @todo mac 测试
if (APPLE)
    message(STATUS "Now is Apple systens.")
    # @todo
elseif (UNIX)
    message(STATUS "Now is UNIX-like OS's.")
    # clang
    find_program(Compiler_clang++ clang++)
    if (NOT Compiler_clang++)
        message(FATAL_ERROR "clang++ not found.\n"
            "Run `sudo apt-get install -y clang clang++` to install.")
    else ()
        message(STATUS "Found clang++ ${Compiler_clang++}")
    endif ()

    set(CMAKE_C_COMPILER clang)
    set(CMAKE_CXX_COMPILER clang++)
    set(CMAKE_READELF readelf)
    set(CMAKE_AR ar)
    set(CMAKE_LINKER ld)
    set(CMAKE_NM nm)
    set(CMAKE_OBJDUMP objdump)
    set(CMAKE_RANLIB ranlib)

    # qemu
    find_program(QEMU qemu-system-x86_64)
    if (NOT QEMU)
        message(FATAL_ERROR "qemu not found.\n"
            "Run `sudo apt-get install -y qemu-system` to install.")
    else ()
        message(STATUS "Found qemu ${QEMU}")
    endif ()
endif ()
