
# This file is a part of SimpleXX/SimpleKernel (https://github.com/SimpleXX/SimpleKernel).
# 
# toolchain_linux_x86_64.cmake for SimpleXX/SimpleKernel.

set(CMAKE_SYSTEM_NAME Linux)

set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

find_program(LinuxX86_64 x86_64-elf-gcc REQUIRED)
if (NOT LinuxX86_64)
    message(FATAL_ERROR "x86_64-elf-gcc not found.\n"
            "run `brew install x86_64-elf-gcc` to install the toolchain")
else ()
    message(STATUS "found x86_64-elf-gcc ${LinuxX86_64}")
endif ()

set(TOOLCHAIN_PREFIX)
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)
set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy)
