
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# add_header.cmake for Simple-XX/SimpleKernel.
# 将头文件路径添加到 _target 的搜索路径中

function(add_header_project _target)
    target_include_directories(${_target} PRIVATE
            ${CMAKE_SOURCE_DIR}/src)
endfunction()

function(add_header_boot _target)
    target_include_directories(${_target} PRIVATE
            ${CMAKE_SOURCE_DIR}/src/boot/include)
endfunction()

function(add_header_libc _target)
    target_include_directories(${_target} PRIVATE
            ${CMAKE_SOURCE_DIR}/src/kernel/libc/include)
endfunction()

function(add_header_libcxx _target)
    target_include_directories(${_target} PRIVATE
            ${CMAKE_SOURCE_DIR}/src/kernel/libcxx/include)
endfunction()

function(add_header_arch _target)
    target_include_directories(${_target} PRIVATE
            ${CMAKE_SOURCE_DIR}/src/kernel/arch)
endfunction()

function(add_header_kernel _target)
    target_include_directories(${_target} PRIVATE
            ${CMAKE_SOURCE_DIR}/src/kernel/include)
endfunction()

function(add_header_driver _target)
    target_include_directories(${_target} PRIVATE
            ${CMAKE_SOURCE_DIR}/src/kernel/driver/include)
endfunction()

function(add_header_3rd _target)
    if (${TARGET_ARCH} STREQUAL "x86_64" OR ${TARGET_ARCH} STREQUAL "aarch64")
        target_include_directories(${_target} PRIVATE
                ${gnu-efi_BINARY_DIR}/inc)
        target_include_directories(${_target} PRIVATE
                ${gnu-efi_BINARY_DIR}/inc/${TARGET_ARCH})
        target_include_directories(${_target} PRIVATE
                ${gnu-efi_BINARY_DIR}/inc/protocol)
    elseif (${TARGET_ARCH} STREQUAL "riscv64")
        target_include_directories(${_target} PRIVATE
                ${opensbi_BINARY_DIR}/include)
    endif ()
endfunction()
