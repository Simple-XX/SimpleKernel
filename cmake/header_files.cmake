
# This file is a part of SimpleXX/SimpleKernel (https://github.com/SimpleXX/SimpleKernel).
# 
# header_files.cmake for SimpleXX/SimpleKernel.

function(target_include_libc_header_files Target)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/libc/include)
endfunction()

function(target_include_kernel_header_files Target)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/kernel/include)
endfunction()

function(target_include_arch_header_files Target)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/arch/${SimpleKernelArch}/)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/arch/${SimpleKernelArch}/gdt/include)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/arch/${SimpleKernelArch}/intr/include)
endfunction()

function(target_include_tests_header_files Target)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/test/include)
endfunction()

function(target_include_ds_alg_header_files Target)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/ds_alg/include)
endfunction()

function(target_include_drv_header_files Target)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/drv/8259A/include)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/drv/clock/include)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/drv/vga/include)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/drv/console/include)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/drv/keyboard/include)
endfunction()

function(target_include_common_header_files Target)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/include)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/include/heap)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/include/mem)
endfunction()
