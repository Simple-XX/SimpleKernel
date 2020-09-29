
# This file is a part of SimpleXX/SimpleKernel (https://github.com/SimpleXX/SimpleKernel).
#
# CMakeLists.txt for SimpleXX/SimpleKernel.

# Set minimum cmake version
cmake_minimum_required(VERSION 3.18)

# skip cmake compiler check
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_CXX_COMPILER_WORKS TRUE)

project(SimpleKernel LANGUAGES C ASM)

if(${SimpleKernel_SOURCE_DIR} STREQUAL ${SimpleKernel_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

# Set debug
set(CMAKE_BUILD_TYPE Debug)

# Set c11
set(CMAKE_C_STANDARD 11)

# Set flags
set(CFLAGS "-std=gnu11 -ffreestanding -nostdinc -nostdlib -fno-exceptions -nostartfiles -fno-builtin -O2 -Wall -Wextra -Wshadow -Wunreachable-code -Winline -Wmissing-prototypes -Wsign-compare -mno-sse -g -ggdb -C -c -m32 -march=corei7")
set(CMAKE_C_FLAGS ${CFLAGS})

# Linker script
set(LINKER_SCRIPT "arch/x86_64/boot/link32.ld")
set(LDFLAGS -A i386 -m elf_i386 -T${LINKER_SCRIPT})

# Kernel name
set(KernelName SimpleKernel.elf)

# Add include files
include_directories(
    ${SimpleKernel_SOURCE_DIR}/test/include
    ${SimpleKernel_SOURCE_DIR}/libc
    ${SimpleKernel_SOURCE_DIR}/kernel/include
    ${SimpleKernel_SOURCE_DIR}/include
    ${SimpleKernel_SOURCE_DIR}/ds_alg
    ${SimpleKernel_SOURCE_DIR}/drv/8259A/include
    ${SimpleKernel_SOURCE_DIR}/drv/clock/include
    ${SimpleKernel_SOURCE_DIR}/drv/console/include
    ${SimpleKernel_SOURCE_DIR}/drv/keyboard/include
    ${SimpleKernel_SOURCE_DIR}/drv/mouse/include
    ${SimpleKernel_SOURCE_DIR}/drv/vga/include
    ${SimpleKernel_SOURCE_DIR}/arch/x86_64/boot/include
    ${SimpleKernel_SOURCE_DIR}/arch/x86_64/gdt/include
    ${SimpleKernel_SOURCE_DIR}/arch/x86_64/intr/include
    ${SimpleKernel_SOURCE_DIR}/arch/x86_64
)

# Add source files
set(source 
    ${SimpleKernel_SOURCE_DIR}/test/test.c
    ${SimpleKernel_SOURCE_DIR}/libc/stdio/printk.c
    ${SimpleKernel_SOURCE_DIR}/libc/stdio/vsprintf.c
    ${SimpleKernel_SOURCE_DIR}/libc/string/string.c
    ${SimpleKernel_SOURCE_DIR}/kernel/multiboot2.c
    ${SimpleKernel_SOURCE_DIR}/kernel/kernel.c
    ${SimpleKernel_SOURCE_DIR}/kernel/elf.c
    ${SimpleKernel_SOURCE_DIR}/kernel/mem/buddy.c
    ${SimpleKernel_SOURCE_DIR}/kernel/mem/first_fit.c
    ${SimpleKernel_SOURCE_DIR}/kernel/mem/slab.c
    ${SimpleKernel_SOURCE_DIR}/ds_alg/LinkedList.c
    ${SimpleKernel_SOURCE_DIR}/drv/8259A/8259A.c
    ${SimpleKernel_SOURCE_DIR}/drv/clock/clock.c
    ${SimpleKernel_SOURCE_DIR}/drv/console/console.c
    ${SimpleKernel_SOURCE_DIR}/drv/keyboard/keyboard.c
    ${SimpleKernel_SOURCE_DIR}/drv/mouse/mouse.c
    ${SimpleKernel_SOURCE_DIR}/arch/x86_64/arch_init.c
    ${SimpleKernel_SOURCE_DIR}/arch/x86_64/boot/bootinit.c
    ${SimpleKernel_SOURCE_DIR}/arch/x86_64/boot/boot.s
    ${SimpleKernel_SOURCE_DIR}/arch/x86_64/debug/debug.c
    ${SimpleKernel_SOURCE_DIR}/arch/x86_64/gdt/gdt.c
    ${SimpleKernel_SOURCE_DIR}/arch/x86_64/gdt/gdt_s.s
    ${SimpleKernel_SOURCE_DIR}/arch/x86_64/heap/heap.c
    ${SimpleKernel_SOURCE_DIR}/arch/x86_64/intr/intr_s.s
    ${SimpleKernel_SOURCE_DIR}/arch/x86_64/intr/intr.c
    ${SimpleKernel_SOURCE_DIR}/arch/x86_64/mm/buffer.c
    ${SimpleKernel_SOURCE_DIR}/arch/x86_64/mm/pmm.c
    ${SimpleKernel_SOURCE_DIR}/arch/x86_64/mm/vmm.c
    ${SimpleKernel_SOURCE_DIR}/arch/x86_64/sched/sched_s.s
    ${SimpleKernel_SOURCE_DIR}/arch/x86_64/sched/sched.c
    ${SimpleKernel_SOURCE_DIR}/arch/x86_64/task/task_s.s
    ${SimpleKernel_SOURCE_DIR}/arch/x86_64/task/task.c
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_EXE_LINKER_FLAGS -T${LINKER_SCRIPT})

add_executable(${KernelName} ${source})
