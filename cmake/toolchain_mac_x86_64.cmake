
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
# 
# toolchain_mac_x86_64.cmake for Simple-XX/SimpleKernel.

set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# GCC
find_program(X86_64 x86_64-elf-gcc REQUIRED)
if (NOT X86_64)
    message(FATAL_ERROR "x86_64-elf-gcc not found.\n"
            "Run `brew install x86_64-elf-gcc` to install the toolchain.")
else ()
    message(STATUS "Found x86_64-elf-gcc ${X86_64}.")
endif ()

set(TOOLCHAIN_PREFIX x86_64-elf-)
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)
set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy)

# Grub
find_program(Grub grub-file)
if (NOT Grub)
    message(FATAL_ERROR "grub-file not found.\n"
            "Use grub4mac.sh to install grub.")
else ()
    message(STATUS "Found grub-file ${Grub}")
endif ()

# bochs
find_program(Bochs bochs REQUIRED)
if (NOT Bochs)
    message(FATAL_ERROR "bochs not found.\n"
            "run `brew install bochs` to install the toolchain")
else ()
    message(STATUS "Found bochs ${Bochs}")
endif ()
