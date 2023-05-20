
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.
# CMake 入口

# 设置最小 cmake 版本
cmake_minimum_required(VERSION 3.13)

# 设置项目名与版本
project(SimpleKernel VERSION 0.0.1)

# 禁止原地编译
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif ()

# 设置辅助 cmake 脚本路径
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

# 导入配置
include(config)
# 引入添加头文件函数
include(header_files)
# 引入添加汇编文件函数
include(find_asm_files)

# 设置工具链
#include(ExternalProject)
# uefi 引导
#ExternalProject_Add(
#        uefi_boot
#        BUILD_ALWAYS 1
#        PREFIX ${CMAKE_BINARY_DIR}/uefi_boot
#        SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/uefi_boot
#        BINARY_DIR ${CMAKE_BINARY_DIR}/uefi_boot
#        CMAKE_ARGS
#            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
#            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_SOURCE_DIR}/cmake/${ARCH}-${CMAKE_HOST_SYSTEM_PROCESSOR}.cmake
#            -DMACHINE=${MACHINE}
#            -B ${CMAKE_BINARY_DIR}/src/boot
#        BUILD_COMMAND ${GENERATOR_COMMAND}
#)
## 内核本体
#ExternalProject_Add(
#        kernel_elf
#        BUILD_ALWAYS 1
#        PREFIX ${CMAKE_BINARY_DIR}/kernel
#        SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/kernel
#        BINARY_DIR ${CMAKE_BINARY_DIR}/kernel
#        CMAKE_ARGS
#            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
#            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_SOURCE_DIR}/cmake/${ARCH}-${CMAKE_HOST_SYSTEM_PROCESSOR}.cmake
#            -DMACHINE=${MACHINE}
#            -B ${CMAKE_BINARY_DIR}/kernel
#        BUILD_COMMAND ${GENERATOR_COMMAND}
#)

# 制作内核
# 将内核调整为 efi
add_custom_target(kernel_efi DEPENDS kernel_elf)
add_custom_command(TARGET kernel_efi
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMENT "Gen kernel.efi"
        COMMAND ${CMAKE_OBJCOPY} --target=efi-app-x86-64 ${KernelName} kernel.efi
            -g -R .comment -R .gnu_debuglink -R .note.gnu.build-id
            -R .gnu.hash -R .plt -R .rela.plt -R .dynstr -R .dynsym -R .rela.dyn
            -S -R .eh_frame -R .gcc_except_table
        )

add_custom_target(image_uefi DEPENDS uefi_boot kernel_efi)
add_custom_command(TARGET image_uefi
        COMMENT "Copying bootloader and kernel"
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/image/efi/boot
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/boot/src/*.efi ${CMAKE_BINARY_DIR}/image/efi/boot/
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/image/efi/rainbow
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/boot/src/*.efi ${CMAKE_BINARY_DIR}/image/efi/rainbow/
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/kernel/src/kernel ${CMAKE_BINARY_DIR}/image/efi/rainbow/
)
# 制作镜像
add_custom_target(image DEPENDS image_uefi)
add_custom_command(TARGET image
        POST_BUILD
        COMMENT "Building UEFI disk image"
        COMMAND dd if=/dev/zero of=${CMAKE_BINARY_DIR}/rainbow-efi-${ARCH}.img bs=1M count=33
        COMMAND mkfs.vfat ${CMAKE_BINARY_DIR}/rainbow-efi-${ARCH}.img -F32
        COMMAND mcopy -s -i ${CMAKE_BINARY_DIR}/rainbow-efi-${ARCH}.img ${CMAKE_BINARY_DIR}/image/* ::
)

# 固件
if (ARCH STREQUAL "riscv64")
elseif (ARCH STREQUAL "x86_64")
    set(FIRMWARE OVMF_${ARCH}.fd)
    file(
            COPY ${CMAKE_SOURCE_DIR}/3rd/ovmf/${FIRMWARE}
            DESTINATION ${CMAKE_BINARY_DIR}/emulation/
    )
    set(QEMU_FIRMWARE -drive if=pflash,format=raw,file=${CMAKE_BINARY_DIR}/emulation/${FIRMWARE})
elseif (ARCH STREQUAL "aarch64")
    # The aarch64 emulation firmware needs to be padded to 64m (don't know why, might have to do with using "-machine virt")
    # We also need a separate file for NVRAM.
    set(FIRMWARE ovmf-aarch64.fd)
    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/emulation/)
    execute_process(COMMAND truncate -s 64m ${CMAKE_BINARY_DIR}/emulation/${FIRMWARE})
    execute_process(COMMAND truncate -s 64m ${CMAKE_BINARY_DIR}/emulation/nvram.img)
    execute_process(COMMAND dd if=${CMAKE_SOURCE_DIR}/third_party/firmware/tianocore/${FIRMWARE} of=${CMAKE_BINARY_DIR}/emulation/${FIRMWARE} conv=notrunc)
    set(QEMU_FIRMWARE -drive if=pflash,format=raw,file=${CMAKE_BINARY_DIR}/emulation/${FIRMWARE},readonly -drive if=pflash,format=raw,file=$(CMAKE_BINARY_DIR)/emulation/nvram.img)
endif ()

# 运行 QEMU
# qemu 配置
set(QEMU_FLAGS -serial stdio -monitor telnet::2333,server,nowait)
# 内核镜像
set(QEMU_IMAGE)

if (ARCH STREQUAL "riscv64")
    set(QEMU_MACHINE -machine virt -m 8G -cpu cortex-a53)
    set(QEMU_DEVICES -device virtio-gpu-pci)
elseif (ARCH STREQUAL "x86_64")
    set(QEMU_MACHINE -machine virt)
    set(QEMU_DEVICES -device virtio-gpu-pci)
    list(APPEND QEMU_FLAGS -accel kvm -boot order=d)
elseif (ARCH STREQUAL "aarch64")
    set(QEMU_MACHINE -machine virt -m 8G -cpu cortex-a53)
    set(QEMU_DEVICES -device virtio-gpu-pci)
endif ()

add_custom_target(run DEPENDS image)
add_custom_command(
        TARGET run
        COMMENT "Run SimpleKernel in qemu."
        COMMAND qemu-system-${ARCH} ${QEMU_FLAGS} ${QEMU_MACHINE} ${QEMU_DEVICES} ${QEMU_FIRMWARE} ${QEMU_IMAGE} #-d int,cpu_reset -monitor telnet:127.0.0.1:55555,server,nowait
)

# 跳转到 SimpleKernel_SOURCE_CODE_DIR 下的 CMakeLists
add_subdirectory(${SimpleKernel_SOURCE_DIR}/src)
