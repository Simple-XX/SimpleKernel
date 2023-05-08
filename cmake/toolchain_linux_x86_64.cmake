
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
# 
# toolchain_linux_x86_64.cmake for Simple-XX/SimpleKernel.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# GCC
find_program(G++ g++)
if (NOT G++)
    message(FATAL_ERROR "g++ not found.\n"
            "Run `sudo apt-get install -y gcc g++` to install.")
else ()
    message(STATUS "Found g++ ${G++}")
endif ()

# xorriso
find_program(XORRISO xorriso)
if (NOT XORRISO)
    message(FATAL_ERROR "xorriso not found.\n"
            "Run `sudo apt-get install -y xorriso` to install.")
else ()
    message(STATUS "Found xorriso ${XORRISO}")
endif ()

# GRUB
find_program(GRUB grub-file)
if (NOT GRUB)
    message(FATAL_ERROR "grub-file not found.\n"
            "Run `sudo apt-get install -y grub2` to install.")
else ()
    message(STATUS "Found grub-file ${GRUB}")
endif ()

# qemu
find_program(QEMU qemu-system-x86_64)
if (NOT QEMU)
    message(FATAL_ERROR "qemu not found.\n"
            "Run `sudo apt-get install -y qemu-system` to install.")
else ()
    message(STATUS "Found qemu ${QEMU}")
endif ()
