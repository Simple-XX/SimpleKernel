
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# compile_config.cmake for Simple-XX/SimpleKernel.
# 配置信息

# 通用宏定义
add_library(compile_definitions INTERFACE)
target_compile_definitions(compile_definitions INTERFACE
        $<$<CONFIG:Release>:SIMPLEKERNEL_RELEASE>
        $<$<CONFIG:Debug>:SIMPLEKERNEL_DEBUG>
)

# 通用编译选项
add_library(compile_options INTERFACE)
target_compile_options(compile_options INTERFACE
        # 如果 CMAKE_BUILD_TYPE 为 Release 则使用 -O3 -Werror，否则使用 -O0 -ggdb
        # -g 在 Debug 模式下由 cmake 自动添加
        $<$<CONFIG:Release>:-O3;-Werror>
        $<$<CONFIG:Debug>:-O0;-ggdb>
        # 打开全部警告
        -Wall
        # 打开额外警告
        -Wextra
        # 不符合规范的代码会警告
        -pedantic
        # 生成位置无关代码
        -fPIC
        # 生成位置无关可执行程序
        -fPIE
        # 禁用运行时类型支持
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
        # 启用 free-standing 环境，该选项隐含了 -fno-builtin
        -ffreestanding
        # 保留帧指针，便于调试和栈回溯
        -fno-omit-frame-pointer
        # 不使用 common 段
        -fno-common

        # 目标平台编译选项
        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},x86_64>:
        # 禁用 red-zone
        -mno-red-zone
        >

        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},riscv64>:
        >

        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},aarch64>:
        # 生成 armv8-a 代码
        -march=armv8-a
        # 针对 cortex-a72 优化代码
        -mtune=cortex-a72
        >

        # gcc 特定选项
        $<$<CXX_COMPILER_ID:GNU>:
        >

        # clang 特定选项
        $<$<CXX_COMPILER_ID:Clang>:
        >

        # 平台相关
        $<$<PLATFORM_ID:Darwin>:
        >
)

# 通用链接选项
add_library(link_options INTERFACE)
target_link_options(link_options INTERFACE
        # 不链接 ctr0 等启动代码
        -nostartfiles
)

# 通用库选项
add_library(link_libraries INTERFACE)
target_link_libraries(link_libraries INTERFACE
        compile_definitions
        compile_options
        link_options
)

add_library(boot_compile_definitions INTERFACE)
target_compile_definitions(boot_compile_definitions INTERFACE
        # 使用 gnu-efi
        GNU_EFI_USE_MS_ABI
)

add_library(boot_compile_options INTERFACE)
target_compile_options(boot_compile_options INTERFACE
        # 使用 2 字节 wchar_t
        -fshort-wchar
        # 允许 wchar_t
        -fpermissive

        # 目标平台编译选项
        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},x86_64>:
        >

        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},aarch64>:
        >
)

add_library(boot_link_options INTERFACE)
target_link_options(boot_link_options INTERFACE
        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},x86_64>:
        # 编译为共享库
        -shared
        # 符号级别绑定
        -Wl,-Bsymbolic
        >

        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},riscv64>:
        # 编译为共享库
        -shared
        # 符号级别绑定
        -Wl,-Bsymbolic
        >

        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},aarch64>:
        -shared
        -Wl,-Bsymbolic
        >
)

add_library(boot_link_libraries INTERFACE)
target_link_libraries(boot_link_libraries INTERFACE
        link_libraries
        boot_compile_definitions
        boot_compile_options
        boot_link_options

        # 目标平台编译选项
        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},x86_64>:
        # 链接 gnu-efi
        gnu-efi-lib
        >

        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},riscv64>:
        # 链接 gnu-efi
        gnu-efi-lib
        >

        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},aarch64>:
        # 链接 gnu-efi
        gnu-efi-lib
        >
)

add_library(kernel_compile_definitions INTERFACE)
target_compile_definitions(kernel_compile_definitions INTERFACE
        USE_NO_RELAX=$<BOOL:${USE_NO_RELAX}>
)

add_library(kernel_compile_options INTERFACE)
target_compile_options(kernel_compile_options INTERFACE
        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},x86_64>:
        # 使用 kernel 内存模型
        -mcmodel=large
        >

        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},riscv64>:
        # 使用 medany 内存模型
        # 代码和数据段可以在任意地址
        -mcmodel=medany
        >
)

add_library(kernel_link_options INTERFACE)
target_link_options(kernel_link_options INTERFACE
        # 链接脚本
        -T ${CMAKE_SOURCE_DIR}/src/kernel/arch/${CMAKE_SYSTEM_PROCESSOR}/link.ld

        # 静态链接
        -static
        # 不链接标准库
        -nostdlib

        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},x86_64>:
        # 设置最大页大小为 0x1000(4096) 字节
        -z max-page-size=0x1000
        >

        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},riscv64>:
        # 禁用 relax 优化
        $<$<BOOL:${USE_NO_RELAX}>:-mno-relax>
        >
)

add_library(kernel_link_libraries INTERFACE)
target_link_libraries(kernel_link_libraries INTERFACE
        link_libraries
        kernel_compile_definitions
        kernel_compile_options
        kernel_link_options

        printf_bare_metal
        dtc-lib

        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},riscv64>:
        opensbi_interface
        >

        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},aarch64>:
        >
)
