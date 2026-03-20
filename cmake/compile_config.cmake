# Copyright The SimpleKernel Contributors

# 通用宏定义
ADD_LIBRARY (compile_definitions INTERFACE)
TARGET_COMPILE_DEFINITIONS (
    compile_definitions
    INTERFACE
        _GLIBCXX_NO_ASSERTIONS
        $<$<CONFIG:Release>:SIMPLEKERNEL_RELEASE>
        $<$<CONFIG:Debug>:SIMPLEKERNEL_DEBUG>
        $<$<CONFIG:Debug>:SIMPLEKERNEL_MIN_LOG_LEVEL=0>
        $<$<BOOL:${SIMPLEKERNEL_MAX_CORE_COUNT}>:SIMPLEKERNEL_MAX_CORE_COUNT=${SIMPLEKERNEL_MAX_CORE_COUNT}>
        $<$<BOOL:${SIMPLEKERNEL_DEFAULT_STACK_SIZE}>:SIMPLEKERNEL_DEFAULT_STACK_SIZE=${SIMPLEKERNEL_DEFAULT_STACK_SIZE}>
        $<$<BOOL:${SIMPLEKERNEL_PER_CPU_ALIGN_SIZE}>:SIMPLEKERNEL_PER_CPU_ALIGN_SIZE=${SIMPLEKERNEL_PER_CPU_ALIGN_SIZE}>
        SIMPLEKERNEL_EARLY_CONSOLE_BASE=${SIMPLEKERNEL_EARLY_CONSOLE_BASE}
        $<$<BOOL:${SIMPLEKERNEL_TICK}>:SIMPLEKERNEL_TICK=${SIMPLEKERNEL_TICK}>)

# 第三方宏定义
ADD_LIBRARY (3rd_compile_definitions INTERFACE)
TARGET_COMPILE_DEFINITIONS (
    3rd_compile_definitions
    INTERFACE ETL_CPP23_SUPPORTED ETL_NO_STD_OSTREAM ETL_VERBOSE_ERRORS
              ETL_NO_CPP_NAN_SUPPORT ETL_FORMAT_NO_FLOATING_POINT)

# 获取 gcc 的 include 路径
EXECUTE_PROCESS (
    COMMAND sh -c "echo | ${CMAKE_CXX_COMPILER} -v -x c -E - 2>&1 | sed -n \
        '/#include <...> search starts here:/,/End of search list./{/^ /p}'"
    OUTPUT_VARIABLE GCC_OUTPUT
    ERROR_VARIABLE GCC_ERROR
    RESULT_VARIABLE GCC_RESULT
    OUTPUT_STRIP_TRAILING_WHITESPACE)
# 检查 gcc 是否成功执行
IF(NOT GCC_RESULT EQUAL 0)
    MESSAGE (FATAL_ERROR "Failed to run ${CMAKE_CXX_COMPILER} -v")
ENDIF()
# 分割路径并生成路径列表
STRING (REPLACE "\n" ";" INCLUDE_PATH_LIST "${GCC_OUTPUT}")
# 使用 `-I` 将路径添加到编译选项中
FOREACH(item ${INCLUDE_PATH_LIST})
    STRING (REGEX REPLACE " " "" CLEAN_PATH ${item})
    LIST (APPEND CROSS_INCLUDE_PATHS "-I${CLEAN_PATH}")
ENDFOREACH()
MESSAGE (STATUS "GCC Include CROSS_INCLUDE_PATHS: ${CROSS_INCLUDE_PATHS}")

# 通用编译选项
ADD_LIBRARY (compile_options INTERFACE)
TARGET_COMPILE_OPTIONS (
    compile_options
    INTERFACE # 如果 CMAKE_BUILD_TYPE 为 Release 则使用 -O3 -Werror，否则使用 -O0 -ggdb -g
              # 在 Debug 模式下由 cmake 自动添加
              $<$<CONFIG:Release>:-O2;-Werror>
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
              # 禁用异常支持
              -fno-exceptions
              # 启用 free-standing 环境，该选项隐含了 -fno-builtin
              -ffreestanding
              # 保留帧指针，便于调试和栈回溯
              -fno-omit-frame-pointer
              # 不使用 common 段
              -fno-common
              # 禁用 new 的异常支持
              -fcheck-new
              # 目标平台编译选项
              $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},riscv64>:
              # 严格对齐
              -mstrict-align
              # 启用 zihintpause 拓展
              -march=rv64gc_zihintpause
              >
              $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},aarch64>:
              # 仅使用通用寄存器
              -mgeneral-regs-only
              # 严格对齐
              -mstrict-align
              # 生成 armv8-a 代码
              -march=armv8-a
              # 针对 cortex-a72 优化代码
              -mtune=cortex-a72
              -mno-outline-atomics
              >
              # 将编译器的 include 路径添加到编译选项中，以便 clang-tidy 使用
              ${CROSS_INCLUDE_PATHS})

# 通用链接选项
ADD_LIBRARY (link_options INTERFACE)
TARGET_LINK_OPTIONS (
    link_options INTERFACE
    # 不链接 ctr0 等启动代码
    -nostartfiles)

# 通用库选项
ADD_LIBRARY (link_libraries INTERFACE)
TARGET_LINK_LIBRARIES (link_libraries INTERFACE compile_definitions
                                                compile_options link_options)

ADD_LIBRARY (kernel_compile_definitions INTERFACE)
TARGET_COMPILE_DEFINITIONS (kernel_compile_definitions
                            INTERFACE USE_NO_RELAX=$<BOOL:${USE_NO_RELAX}>)

ADD_LIBRARY (kernel_compile_options INTERFACE)
TARGET_COMPILE_OPTIONS (
    kernel_compile_options
    INTERFACE
        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},aarch64>:
        # 使用 large 内存模型
        # https://gcc.gnu.org/onlinedocs/gcc/AArch64-Options.html#index-mcmodel_003dlarge
        # -mcmodel=large
        >
        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},riscv64>:
        # 使用 medany 内存模型 代码和数据段可以在任意地址
        # https://gcc.gnu.org/onlinedocs/gcc/RISC-V-Options.html#index-mcmodel_003dlarge-2
        -mcmodel=medany
        >)

ADD_LIBRARY (kernel_link_options INTERFACE)
TARGET_LINK_OPTIONS (
    kernel_link_options
    INTERFACE
    # 链接脚本
    -T
    ${CMAKE_SOURCE_DIR}/src/arch/${CMAKE_SYSTEM_PROCESSOR}/link.ld
    # 静态链接
    -static
    # 不链接标准库
    -nostdlib
    $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},riscv64>:
    # 禁用 relax 优化
    $<$<BOOL:${USE_NO_RELAX}>:-mno-relax>
    >)

ADD_LIBRARY (kernel_link_libraries INTERFACE)
TARGET_LINK_LIBRARIES (
    kernel_link_libraries
    INTERFACE link_libraries
              kernel_compile_definitions
              kernel_compile_options
              kernel_link_options
              3rd_compile_definitions
              dtc-lib
              cpu_io
              bmalloc
              MPMCQueue
              fatfs_lib
              etl::etl
              gcc
              $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},riscv64>:
              opensbi_interface
              >
              $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},aarch64>:
              teec
              >)
