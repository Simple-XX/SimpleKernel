
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# compile_config.cmake for Simple-XX/SimpleKernel.
# 配置信息

# 通用编译选项
list(APPEND COMMON_COMPILE_OPTIONS
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
        -fno-rtti
        # 禁用异常支持
        -fno-exceptions
        # 启用 free-standing 环境，该选项隐含了 -fno-builtin
        -ffreestanding
        # 保留帧指针，便于调试和栈回溯
        -fno-omit-frame-pointer
        # 禁用栈保护器
        -fno-stack-protector
        # 禁用栈检查
        -fno-stack-check

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
list(APPEND COMMON_LINK_OPTIONS
        # 不链接 ctr0 等启动代码
        -nostartfiles
)

# 通用库选项
list(APPEND COMMON_LINK_LIB
)

# 通用宏定义
list(APPEND COMMON_DEFINITIONS
)

list(APPEND DEFAULT_BOOT_DEFINITIONS
        ${COMMON_DEFINITIONS}

        # 使用 gnu-efi
        GNU_EFI_USE_MS_ABI
)

list(APPEND DEFAULT_BOOT_COMPILE_OPTIONS
        ${COMMON_COMPILE_OPTIONS}
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

list(APPEND DEFAULT_BOOT_LINK_OPTIONS
        ${COMMON_LINK_OPTIONS}

        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},x86_64>:
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

list(APPEND DEFAULT_BOOT_LINK_LIB
        ${COMMON_LINK_LIB}
        # 目标平台编译选项
        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},x86_64>:
        # 链接 gnu-efi
        ${gnu-efi_BINARY_DIR}/gnuefi/reloc_${CMAKE_SYSTEM_PROCESSOR}.o
        ${gnu-efi_BINARY_DIR}/gnuefi/crt0-efi-${CMAKE_SYSTEM_PROCESSOR}.o
        ${gnu-efi_BINARY_DIR}/gnuefi/libgnuefi.a
        ${gnu-efi_BINARY_DIR}/lib/libefi.a
        >

        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},aarch64>:
        # 链接 gnu-efi
        ${gnu-efi_BINARY_DIR}/gnuefi/reloc_${CMAKE_SYSTEM_PROCESSOR}.o
        ${gnu-efi_BINARY_DIR}/gnuefi/crt0-efi-${CMAKE_SYSTEM_PROCESSOR}.o
        ${gnu-efi_BINARY_DIR}/gnuefi/libgnuefi.a
        ${gnu-efi_BINARY_DIR}/lib/libefi.a
        >
)

list(APPEND DEFAULT_KERNEL_DEFINITIONS
        ${COMMON_DEFINITIONS}

        USE_NO_RELAX=$<BOOL:${USE_NO_RELAX}>
)

list(APPEND DEFAULT_KERNEL_COMPILE_OPTIONS
        ${COMMON_COMPILE_OPTIONS}

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

list(APPEND DEFAULT_KERNEL_LINK_OPTIONS
        ${COMMON_LINK_OPTIONS}

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

list(APPEND DEFAULT_KERNEL_LINK_LIB
        ${COMMON_LINK_LIB}

        printf_bare_metal

        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},riscv64>:
        opensbi_interface
        fdt_parser
        >

        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},aarch64>:
        fdt_parser
        >
)

# 编译依赖
if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    list(APPEND COMPILE_DEPENDS
            gnu-efi
            printf_bare_metal
    )
elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "riscv64")
    list(APPEND COMPILE_DEPENDS
            opensbi
            opensbi_interface
            printf_bare_metal
            fdt_parser
    )
elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
    list(APPEND COMPILE_DEPENDS
            gnu-efi
            fdt_parser
    )
endif ()
