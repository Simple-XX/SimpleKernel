
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
# 
# x86_64-x86_64.cmake for Simple-XX/SimpleKernel.

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# todo mac 支持

# GCC
find_program(G++ g++)
if (NOT G++)
    message(FATAL_ERROR "g++ not found.\n"
            "Run `sudo apt-get install -y gcc g++` to install.")
else ()
    message(STATUS "Found g++ ${G++}")
endif ()

# qemu
find_program(QEMU qemu-system-x86_64)
if (NOT QEMU)
    message(FATAL_ERROR "qemu not found.\n"
            "Run `sudo apt-get install -y qemu-system` to install.")
else ()
    message(STATUS "Found qemu ${QEMU}")
endif ()

set(CMAKE_C_FLAGS
        "${CMAKE_C_FLAGS} -march=corei7 -mtune=corei7 -m64 -mno-red-zone")
set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -march=corei7 -mtune=corei7 -m64 -mno-red-zone")
