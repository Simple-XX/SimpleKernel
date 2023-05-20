
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
# Based on https://github.com/SynestiaOS/SynestiaOS
# header_files.cmake for Simple-XX/SimpleKernel.

function(target_include_libc_header_files Target)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_DIR}/src/libc/include)
endfunction()

function(target_include_libcxx_header_files Target)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_DIR}/src/libcxx/include)
endfunction()

function(target_include_kernel_header_files Target)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_DIR}/src/kernel/include)
endfunction()

function(target_include_arch_header_files Target)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_DIR}/src/arch/${SimpleKernelArch}/)
    if (SimpleKernelArch STREQUAL "ia32/i386" OR SimpleKernelArch STREQUAL "ia32/x86_64")
        target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_DIR}/src/arch/ia32)
        target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_DIR}/src/arch/ia32/port/include)
    elseif (SimpleKernelArch STREQUAL riscv64)
        target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_DIR}/src/arch/${SimpleKernelArch}/opensbi/include)
    endif()
endfunction()

function(target_include_common_header_files Target)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_DIR}/src/include)
endfunction()

function(target_include_drv_header_files Target)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_DIR}/src/drv/tui/include)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_DIR}/src/drv/uart/include)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_DIR}/src/drv/opensbi/include)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_DIR}/src/drv/sbi_console/include)
endfunction()

function(target_include_efi_header_files Target)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_DIR}/src/gnu-efi/inc)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_DIR}/src/gnu-efi/inc/x86_64)
    target_include_directories(${Target} PRIVATE ${SimpleKernel_SOURCE_DIR}/src/gnu-efi/inc/protocol)
endfunction()
