
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
# 
# toolchain_mac_x86_64.cmake for Simple-XX/SimpleKernel.

set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

find_program(BREW brew)
if (NOT BREW)
    message(FATAL_ERROR "brew not found.\n"
            "Run `/bin/bash -c \"$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)\"` to install linuxbrew")
else ()
    message(STATUS "Found brew ${BREW}")
endif ()

# GCC
find_program(GCC x86_64-elf-gcc)
if (NOT GCC)
    message(FATAL_ERROR "x86_64-elf-gcc not found.\n"
            "Run `brew install x86_64-elf-gcc` to install the toolchain.")
else ()
    message(STATUS "Found x86_64-elf-gcc ${GCC}.")
endif ()

set(TOOLCHAIN_PREFIX x86_64-elf-)
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)

# xorriso
find_program(Xorriso xorriso)
if (NOT Xorriso)
    message(FATAL_ERROR "xorriso not found.\n"
            "run `brew install xorriso` to install the toolchain")
else ()
    message(STATUS "Found xorriso ${Xorriso}")
endif ()

# GRUB
find_program(GRUB grub-file)
if (NOT GRUB)
    message(FATAL_ERROR "grub-file not found.\n"
            "Use grub4mac.sh to install grub.")
else ()
    message(STATUS "Found grub-file ${GRUB}")
endif ()

# bochs
find_program(BOCHS bochs)
if (NOT BOCHS)
    message(FATAL_ERROR "bochs not found.\n"
            "run `brew install bochs` to install the toolchain")
else ()
    message(STATUS "Found bochs ${BOCHS}")
endif ()

# qemu
find_program(QEMU qemu-system-x86_64)
if (NOT QEMU)
    message(FATAL_ERROR "qemu not found.\n"
            "Please install qemu first.")
else ()
    message(STATUS "Found qemu ${QEMU}")
endif ()
