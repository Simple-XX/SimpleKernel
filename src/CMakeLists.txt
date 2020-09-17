
# This file is a part of SimpleXX/SimpleKernel (https://github.com/SimpleXX/SimpleKernel).
#
# CMakeLists.txt for SimpleXX/SimpleKernel.

# Set minimum cmake version
cmake_minimum_required(VERSION 3.18)

project(SimpleKernel LANGUAGES C ASM)

if(${SimpleKernel_SOURCE_DIR} STREQUAL ${SimpleKernel_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

# Set debug
set(CMAKE_BUILD_TYPE Debug)

# Set c11
set(CMAKE_C_STANDARD 11)

# Set toolchain
set(TARGET x86_64-elf)
set(CC ${TARGET}-gcc)
set(AS ${TARGET}-as)
set(LD ${TARGET}-ld)
set(AR ${TARGET}-ar)
set(OBJCPY ${TARGET}-objcopy)
set(STRIP ${TARGET}-strip)
set(NM ${TARGET}-gcc-nm)
set(READELF ${TARGET}-readelf)

# Set flags
set(CFLAGS -std=gnu11 -ffreestanding -nostdinc -nostdlib -fno-exceptions -nostartfiles
            -fno-builtin -O2 -Wall -Wextra -Wshadow -Wunreachable-code -Winline
            -Wmissing-prototypes -Wsign-compare -mno-sse -g -ggdb -C -c
            -m32)
set(ASFLAGS -march=corei7 --32)
# Linker script
set(LINKER_SCRIPT link32.ld)
set(LDFLAGS -A i386 -m elf_i386 -T${LINKER_SCRIPT})

# Save all .o file
set_property(GLOBAL PROPERTY objs)

# Compile command
function(c2o cfiles)
    # get_property(tmp GLOBAL PROPERTY objs)
    foreach(cfile ${cfiles})
        execute_process(
            COMMAND "${CC} ${CFLAGS} ${cfile} -o test.o")
        # set(tmp "${tmp} ${cfiles}")
        # message(STATUS ${cfile})
    endforeach()
    # set_property(GLOBAL PROPERTY objs "${tmp}")
endfunction()

function(s2o sfiles)
    get_property(tmp GLOBAL PROPERTY objs)
    foreach(sfile ${sfiles})
        execute_process(
            COMMAND ${AS} ${ASFLAGS} sfile)
        set(tmp "${tmp} ${sfiles}") 
        message(${sfile})
    endforeach()
    set_property(GLOBAL PROPERTY objs "${tmp}")
endfunction()

# Link objs together
function(o2elf)
    execute_process(
        COMMAND ${LD} ${LDFLAGS} ${objs})
    message(${objs})
endfunction()
# Kernel name
set(KernelName SimpleKernel.elf)

# add_subdirectory(${SimpleKernel_SOURCE_DIR}/arch)
# add_subdirectory(${SimpleKernel_SOURCE_DIR}/drv)
# add_subdirectory(${SimpleKernel_SOURCE_DIR}/ds_alg)
# add_subdirectory(${SimpleKernel_SOURCE_DIR}/include)
# add_subdirectory(${SimpleKernel_SOURCE_DIR}/kernel)
# add_subdirectory(${SimpleKernel_SOURCE_DIR}/libc)
add_subdirectory(${SimpleKernel_SOURCE_DIR}/test)

# dump_cmake_variables()