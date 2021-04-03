
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
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/arch/${SimpleKernelArch}/port/include)
endfunction()

function(target_include_common_header_files Target)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/include)
endfunction()

function(target_include_drv_header_files Target)
    if (SimpleKernelArch STREQUAL i386 OR SimpleKernelArch STREQUAL x86_64)
        target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/drv/tui/include)
    elseif (SimpleKernelArch STREQUAL raspi2)
        target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_CODE_DIR}/drv/uart/include)
    endif ()
endfunction()
