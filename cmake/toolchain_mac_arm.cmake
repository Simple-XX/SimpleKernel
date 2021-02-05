
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
# 
# toolchain_mac_arm.cmake for Simple-XX/SimpleKernel.

set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR ARM)

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
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)
set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy)

# qemu
find_program(Qemu qemu-system-aarch64)
if (NOT Qemu)
    message(FATAL_ERROR "qemu not found.\n"
            "Please install qemu first.")
else ()
    message(STATUS "Found qemu ${Qemu}")
endif ()
