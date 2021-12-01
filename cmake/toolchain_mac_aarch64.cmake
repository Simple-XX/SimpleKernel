
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
# 
# toolchain_mac_aarch64.cmake for Simple-XX/SimpleKernel.

set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR AARCH64)

set(arm_gcc_toolchain_path ${CMAKE_CURRENT_SOURCE_DIR}/../../../tools/aarch64-unknown-linux-gnu.tar.xz)
set(arm_gcc_toolchain_dir ${CMAKE_CURRENT_SOURCE_DIR}/../../../tools/aarch64-unknown-linux-gnu)
set(arm_gcc_toolchain_url https://github.com/thinkski/osx-arm-linux-toolchains/releases/download/8.3.0/aarch64-unknown-linux-gnu.tar.xz)

# GCC
find_program(GCC ${CMAKE_CURRENT_SOURCE_DIR}/../../../tools/aarch64-unknown-linux-gnu/bin/aarch64-unknown-linux-gnu-g++)
if (NOT GCC)
    message("aarch64-unknown-linux-gnu-g++ not found.\n"
            "install ...")
    file(DOWNLOAD ${arm_gcc_toolchain_url} ${arm_gcc_toolchain_path}
        EXPECTED_MD5 a8acb994a78185191e6be31f3573cec0
        TLS_VERIFY ON
        SHOW_PROGRESS)
    execute_process(COMMAND
        ${CMAKE_COMMAND} -E tar xvzf ${arm_gcc_toolchain_path}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../tools)
else ()
    message(STATUS "Found aarch64-unknown-linux-gnu-g++ ${GCC}")
endif ()

set(TOOLCHAIN_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/../../../tools/aarch64-unknown-linux-gnu/bin/aarch64-unknown-linux-gnu-)
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
