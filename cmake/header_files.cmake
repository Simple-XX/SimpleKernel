
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
# Based on https://github.com/SynestiaOS/SynestiaOS
# header_files.cmake for Simple-XX/SimpleKernel.

function(target_include_libc_header_files Target)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/libc/include)
endfunction()

function(target_include_libcxx_header_files Target)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/libcxx/include)
endfunction()

function(target_include_kernel_header_files Target)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/kernel/include)
endfunction()

function(target_include_arch_header_files Target)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/arch/${SimpleKernelArch}/)
    if (SimpleKernelArch STREQUAL "ia32/i386" OR SimpleKernelArch STREQUAL "ia32/x86_64")
        target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/arch/ia32)
        target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/arch/ia32/multiboot2/include)
        target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/arch/ia32/port/include)
        target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/arch/${SimpleKernelArch}/gdt/include)
    elseif (SimpleKernelArch STREQUAL riscv64)
        target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/arch/${SimpleKernelArch}/opensbi/include)
    endif()
endfunction()


function(target_include_common_header_files Target)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/include)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/include/mem)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/include/heap)
endfunction()

function(target_include_drv_header_files Target)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/drv/dtb/include)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/drv/multiboot2/include)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/drv/tui/include)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/drv/uart/include)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/drv/opensbi/include)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/drv/sbi_console/include)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/drv/dtb/include)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/drv/multiboot2/include)
endfunction()
