
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
# 
# toolchain_win_x86_64.cmake for Simple-XX/SimpleKernel.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# linuxbrew
find_program(Brew brew REQUIRED)
if (NOT Brew)
    message(FATAL_ERROR "brew not found.\n"
            "run `/bin/bash -c \"$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)\"` to install linuxbrew")
else ()
    message(STATUS "Found brew ${Brew}")
endif ()

# GCC
find_program(X86_64 x86_64-elf-gcc REQUIRED)
if (NOT X86_64)
    message(FATAL_ERROR "x86_64-elf-gcc not found.\n"
            "run `brew install x86_64-elf-gcc` to install the toolchain")
else ()
    message(STATUS "Found x86_64-elf-gcc ${X86_64}")
endif ()

set(TOOLCHAIN_PREFIX x86_64-elf-)
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy)

# xorriso
find_program(Xorriso xorriso)
if (NOT Xorriso)
    message(FATAL_ERROR "xorriso not found.\n"
            "run `brew install xorriso` to install the toolchain")
else ()
    message(STATUS "Found xorriso ${Xorriso}")
endif ()

# Grub
find_program(Grub grub-file)
if (NOT Grub)
    message(FATAL_ERROR "grub-file not found.\n"
            "Please install grub2 common first.")
else ()
    message(STATUS "Found grub-file ${Grub}")
endif ()

# bochs
find_program(Bochs bochs)
if (NOT Bochs)
    message(FATAL_ERROR "bochs not found.\n"
            "run `brew install bochs` to install the toolchain")
else ()
    message(STATUS "Found bochs ${Bochs}")
endif ()
