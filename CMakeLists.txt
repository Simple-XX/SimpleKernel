
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
include(ExternalProject)

# 内核本体
ExternalProject_Add(
        kernel
        BUILD_ALWAYS 1
        PREFIX ${CMAKE_BINARY_DIR}
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/src
        BINARY_DIR ${CMAKE_BINARY_DIR}/src
        CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_SOURCE_DIR}/cmake/${ARCH}-${CMAKE_HOST_SYSTEM_PROCESSOR}.cmake
        -DMACHINE=${MACHINE}
        -DARCH=${ARCH}
        -DCMAKE_C_COMPILER_WORKS=${CMAKE_C_COMPILER_WORKS}
        -DCMAKE_CXX_COMPILER_WORKS=${CMAKE_CXX_COMPILER_WORKS}
        -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=${CMAKE_FIND_ROOT_PATH_MODE_PROGRAM}
        -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=${CMAKE_FIND_ROOT_PATH_MODE_LIBRARY}
        -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=${CMAKE_FIND_ROOT_PATH_MODE_INCLUDE}
        -DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=${CMAKE_FIND_ROOT_PATH_MODE_PACKAGE}
        -DCMAKE_C_STANDARD=${CMAKE_C_STANDARD}
        -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
        -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
        -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
        -DCMAKE_ASM_FLAGS=${CMAKE_ASM_FLAGS}
        -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
        -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
        -DKernelName=${KernelName}
        BUILD_COMMAND ${GENERATOR_COMMAND}
        INSTALL_COMMAND ""
)

# objcopy --only-keep-debug foo foo.dbg 创建一个包含调试信息的文件
add_custom_target(kernel_debug DEPENDS kernel)
add_custom_command(TARGET kernel_debug
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMAND ${CMAKE_OBJCOPY} --only-keep-debug ${KernelName} ${KernelName}.debug
        COMMENT "Generating kernel debug...")

# objcopy --strip-debug foo 创建一个去掉调试信息的( strip 的)可执行文件
# objcopy --add-gnu-debuglink=foo.dbg foo 为 strip 的文件添加调试信息链接
add_custom_target(kernel_raw DEPENDS kernel)
add_custom_command(TARGET kernel_raw
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMAND ${CMAKE_OBJCOPY} --strip-debug ${KernelName}
        COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=${KernelName}.debug ${KernelName}
        COMMENT "Generating raw kernel...")

# nm
add_custom_target(nm DEPENDS kernel)
add_custom_command(TARGET nm
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMAND ${CMAKE_NM} -a -A -C -l -n -s --special-syms --synthetic ${KernelName}
        > ${KernelName}.nm
        COMMENT "nm...")

# readelf -a
add_custom_target(readelf DEPENDS kernel)
add_custom_command(TARGET readelf
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMAND ${CMAKE_READELF} -a ${KernelName} > ${KernelName}.readelf || (exit 0)
        COMMENT "readelf...")

# objdump -D
add_custom_target(objdump DEPENDS kernel)
add_custom_command(TARGET objdump
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMAND ${CMAKE_OBJDUMP} -D ${KernelName} > ${KernelName}.disassembly
        COMMENT "Disassembling...")

# 制作内核
# 将内核调整为 efi
add_custom_target(kernel_efi DEPENDS kernel)
add_custom_command(TARGET kernel_efi
        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMENT "Gen kernel.efi"
        COMMAND ${CMAKE_OBJCOPY} --target=efi-app-x86-64 ${KernelName} kernel.efi
        -g -R .comment -R .gnu_debuglink -R .note.gnu.build-id
        -R .gnu.hash -R .plt -R .rela.plt -R .dynstr -R .dynsym -R .rela.dyn
        -S -R .eh_frame -R .gcc_except_table
        )

add_custom_target(image_uefi DEPENDS kernel_efi)
add_custom_command(TARGET image_uefi
        COMMENT "Copying bootloader and kernel"
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/image/efi/boot
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/bin/*.efi ${CMAKE_BINARY_DIR}/image/efi/boot/
        )
# 制作镜像
add_custom_target(image DEPENDS image_uefi)
add_custom_command(TARGET image
        POST_BUILD
        COMMENT "Building UEFI disk image"
        COMMAND dd if=/dev/zero of=${CMAKE_BINARY_DIR}/kernel.img bs=1M count=33
        COMMAND mkfs.vfat ${CMAKE_BINARY_DIR}/kernel.img -F32
        COMMAND mcopy -s -i ${CMAKE_BINARY_DIR}/kernel.img ${CMAKE_BINARY_DIR}/image/* ::
        )

# 固件
if (ARCH STREQUAL "riscv64")
    set(FIRMWARE fw_jump.elf)
    file(
            COPY ${CMAKE_SOURCE_DIR}/3rd/opensbi/build/platform/generic/firmware/${FIRMWARE}
            DESTINATION ${CMAKE_BINARY_DIR}/firmware
    )
    set(QEMU_FIRMWARE -bios ${CMAKE_BINARY_DIR}/firmware/${FIRMWARE})
elseif (ARCH STREQUAL "x86_64")
    set(FIRMWARE OVMF_${ARCH}.fd)
    file(
            COPY ${CMAKE_SOURCE_DIR}/3rd/ovmf/${FIRMWARE}
            DESTINATION ${CMAKE_BINARY_DIR}/firmware
    )
    set(QEMU_FIRMWARE -bios ${CMAKE_BINARY_DIR}/firmware/${FIRMWARE})
elseif (ARCH STREQUAL "aarch64")
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
if (CMAKE_BUILD_TYPE STREQUAL Debug)
#        set(QEMU_DEBUG -S -gdb tcp::1234)
endif ()

if (ARCH STREQUAL "riscv64")
    set(QEMU_MACHINE -machine virt)
    set(QEMU_DEVICES -nographic)
    set(QEMU_IMAGE -kernel ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${KernelName})
elseif (ARCH STREQUAL "x86_64")
    set(QEMU_MACHINE -m 128M)
    set(QEMU_DEVICES -net none)
    set(QEMU_IMAGE -hda fat:rw:${CMAKE_BINARY_DIR}/image/)
    list(APPEND QEMU_FLAGS)
elseif (ARCH STREQUAL "aarch64")
    set(QEMU_MACHINE -machine virt -cpu cortex-a72)
    set(QEMU_DEVICES -net none)
    #    set(QEMU_IMAGE -hda fat:rw:${CMAKE_BINARY_DIR}/image/)
    set(QEMU_IMAGE -drive format=raw,file=${CMAKE_BINARY_DIR}/kernel.img)
endif ()

add_custom_target(run DEPENDS image_uefi)
add_custom_command(
        TARGET run
        COMMENT "Run SimpleKernel in qemu."
        COMMAND qemu-system-${ARCH} ${QEMU_FLAGS} ${QEMU_MACHINE} ${QEMU_DEVICES} ${QEMU_FIRMWARE} ${QEMU_IMAGE} ${QEMU_DEBUG}
)
