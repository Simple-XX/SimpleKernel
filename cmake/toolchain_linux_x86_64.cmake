
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
# 
# toolchain_linux_x86_64.cmake for Simple-XX/SimpleKernel.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# GCC
find_program(G++ g++)
if (NOT G++)
    message(FATAL_ERROR "g++ not found.\n"
            "Run `sudo apt-get install -y gcc g++` to install the toolchain")
else ()
    message(STATUS "Found g++ ${G++}")
endif ()

# xorriso
find_program(XORRISO xorriso)
if (NOT XORRISO)
    message(FATAL_ERROR "xorriso not found.\n"
            "Run `brew install xorriso` to install the toolchain")
else ()
    message(STATUS "Found xorriso ${XORRISO}")
endif ()

# GRUB
find_program(GRUB grub-file)
if (NOT GRUB)
    message(FATAL_ERROR "grub-file not found.\n"
            "Please install grub2 common first.")
else ()
    message(STATUS "Found grub-file ${GRUB}")
endif ()

# bochs
find_program(BOCHS bochs)
if (NOT BOCHS)
    message(FATAL_ERROR "bochs not found.\n"
            "Please install bochs and bochs-x first.")
else ()
    message(STATUS "Found bochs ${BOCHS}")
endif ()
