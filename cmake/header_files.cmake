
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.
# 将头文件路径添加到 Target 的搜索路径中

function(target_include_arch_header_files Target)
    target_include_directories(${Target} PRIVATE ${CMAKE_SOURCE_DIR}/arch/${SimpleKernelArch})
endfunction()

function(target_include_kernel_header_files Target)
    target_include_directories(${Target} PRIVATE ${CMAKE_SOURCE_DIR}/kernel/include)
endfunction()

function(target_include_gnu_efi_header_files Target)
    target_include_directories(${Target} PRIVATE ${CMAKE_SOURCE_DIR}/../3rd/gnu-efi/inc)
endfunction()

function(target_include_posix_uefi_header_files Target)
    target_include_directories(${Target} PRIVATE ${CMAKE_SOURCE_DIR}/../../3rd/posix-uefi/uefi)
endfunction()
