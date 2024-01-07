
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

# 设置最小 cmake 版本
cmake_minimum_required(VERSION 3.27 FATAL_ERROR)

# 设置项目名与版本
project(
        SimpleKernel
        VERSION 0.0.1
)

# 禁止原地编译
if (${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
    # 如果你看到这句话，cmake 此时已经在根目录下生成了一些临时文件，你需要删除它们
    # CMakeFiles, CMakeCache.txt
    message(
            FATAL_ERROR
            "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
    )
endif ()

# 设置辅助 cmake 脚本路径
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# 导入项目配置
include(project_config)
# 导入头文件处理
include(add_header)
# 导入函数
include(functions)

# 导入第三方依赖
include(3rd)

# 导入编译配置
include(compile_config)

# qemu 参数设置
list(APPEND QEMU_FLAGS
        # 使用标准输出显示
        -serial stdio
        # 启动 telnet 服务，使用 2333 端口，不等待连接
        -monitor ${QEMU_MONITOR_ARG}
)
# 目标平台参数
if (TARGET_ARCH STREQUAL "x86_64")
    list(APPEND QEMU_FLAGS
            -m 128M
            -net none
            -bios ${ovmf_BINARY_DIR}/OVMF_${TARGET_ARCH}.fd
            -hda fat:rw:./image/
    )
elseif (TARGET_ARCH STREQUAL "riscv64")
    list(APPEND QEMU_FLAGS
            -machine virt
            -nographic
            # 可选项，qemu7.0 自带了 opensbi1.0
            -bios ${opensbi_BINARY_DIR}/platform/generic/firmware/fw_jump.elf
            -kernel $<TARGET_FILE:kernel>
    )
elseif (TARGET_ARCH STREQUAL "aarch64")
    list(APPEND QEMU_FLAGS
            -machine virt
            -cpu cortex-a57
            -m 128M
            -net none
            -bios ${ovmf_BINARY_DIR}/OVMF_${TARGET_ARCH}.fd
            -hda fat:rw:./image/
    )
endif ()

# 添加要编译的目录
add_subdirectory(${PROJECT_SOURCE_DIR}/src)
add_subdirectory(${PROJECT_SOURCE_DIR}/test)
add_subdirectory(${PROJECT_SOURCE_DIR}/doc)

# 添加 run 和 debug target
add_run_target(NAME run
        DEPENDS $<$<STREQUAL:${TARGET_ARCH},x86_64>:boot> $<$<STREQUAL:${TARGET_ARCH},aarch64>:boot> kernel ${RUN_DEPENDS} ${DEBUG_DEPENDS}
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        TARGET ${TARGET_ARCH}
        BOOT ${PROJECT_BINARY_DIR}/src/boot/boot.efi
        KERNEL $<TARGET_FILE:kernel>
        QEMU_FLAGS ${QEMU_FLAGS}
)
