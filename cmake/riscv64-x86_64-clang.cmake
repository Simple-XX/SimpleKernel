
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
# 
# riscv64-x86_64-clang.cmake for Simple-XX/SimpleKernel.

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv64)

# clang++
find_program(CLANG clang++)
if (NOT CLANG)
    message(FATAL_ERROR "clang++ not found.\n"
            "Run `sudo apt install -y clang` to install.")
else ()
    message(STATUS "Found clang++ ${CLANG}.")
endif ()

set(TOOLCHAIN_PREFIX   llvm-)
set(TOOLCHAIN_SUFFIX   -15)
set(CMAKE_C_COMPILER   ${CLANG})
set(CMAKE_CXX_COMPILER ${CLANG})
set(CMAKE_READELF      ${TOOLCHAIN_PREFIX}readelf${TOOLCHAIN_SUFFIX})
set(CMAKE_AR           ${TOOLCHAIN_PREFIX}ar${TOOLCHAIN_SUFFIX})
set(CMAKE_LINKER       ${TOOLCHAIN_PREFIX}ld${TOOLCHAIN_SUFFIX})
set(CMAKE_NM           ${TOOLCHAIN_PREFIX}nm${TOOLCHAIN_SUFFIX})
set(CMAKE_OBJDUMP      ${TOOLCHAIN_PREFIX}objdump${TOOLCHAIN_SUFFIX})
set(CMAKE_RANLIB       ${TOOLCHAIN_PREFIX}ranlib${TOOLCHAIN_SUFFIX})

# qemu
find_program(QEMU qemu-system-riscv64)
if (NOT QEMU)
    message(FATAL_ERROR "qemu not found.\n"
            "Run `sudo apt-get install -y qemu-system` to install.")
else ()
    message(STATUS "Found qemu ${QEMU}")
endif ()
