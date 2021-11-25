
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
# 
# toolchain_mac_arm.cmake for Simple-XX/SimpleKernel.

set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR ARM)

find_program(BREW brew)
if (NOT BREW)
    message(FATAL_ERROR "brew not found.\n"
            "Run `/bin/bash -c \"$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)\"` to install linuxbrew")
else ()
    message(STATUS "Found brew ${BREW}")
endif ()

# GCC
find_program(GCC arm-none-eabi-gcc)
if (NOT GCC)
    message(FATAL_ERROR "arm-none-eabi-gcc not found.\n"
            "Run `brew install arm-none-eabi-gcc` to install the toolchain")
else ()
    message(STATUS "Found arm-none-eabi-gcc ${GCC}")
endif ()

set(TOOLCHAIN_PREFIX arm-none-eabi-)
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)

# qemu
find_program(QEMU qemu-system-aarch64)
if (NOT QEMU)
    message(FATAL_ERROR "qemu not found.\n"
            "Please install qemu first.")
else ()
    message(STATUS "Found qemu ${QEMU}")
endif ()
