
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
# 
# toolchain_linux_arm.cmake for Simple-XX/SimpleKernel.

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR ARM)

# linuxbrew
find_program(Brew brew REQUIRED)
if (NOT Brew)
    message(FATAL_ERROR "brew not found.\n"
            "Run `/bin/bash -c \"$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)\"` to install linuxbrew")
else ()
    message(STATUS "Found brew ${Brew}")
endif ()

# GCC
find_program(Arm arm-none-eabi-gcc REQUIRED)
if (NOT Arm)
    message(FATAL_ERROR "arm-none-eabi-gcc not found.\n"
            "Run `brew install arm-none-eabi-gcc` to install the toolchain")
else ()
    message(STATUS "Found arm-none-eabi-gcc ${Arm}")
endif ()

set(TOOLCHAIN_PREFIX arm-none-eabi-)
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy)

# Grub
find_program(Grub grub-file)
if (NOT Grub)
    message(FATAL_ERROR "grub-file not found.\n"
            "Please install grub2 common first.")
else ()
    message(STATUS "Found grub-file ${Grub}")
endif ()

# qemu
find_program(Qemu qemu-system-arm)
if (NOT Qemu)
    message(FATAL_ERROR "qemu not found.\n"
            "Please install qemu first.")
else ()
    message(STATUS "Found qemu ${Qemu}")
endif ()
