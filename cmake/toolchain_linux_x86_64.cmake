
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
# 
# toolchain_linux_x86_64.cmake for Simple-XX/SimpleKernel.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# linuxbrew
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
            "Run `brew install x86_64-elf-gcc` to install the toolchain")
else ()
    message(STATUS "Found x86_64-elf-gcc ${GCC}")
endif ()

set(TOOLCHAIN_PREFIX x86_64-elf-)
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy)

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
