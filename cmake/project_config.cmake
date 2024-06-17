
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# project_config.cmake for Simple-XX/SimpleKernel.
# 配置信息

# 设置 cmake 目标环境根目录
# @todo 设置目录
list(APPEND CMAKE_FIND_ROOT_PATH
        /usr/x86_64-linux-gnu
        /usr/riscv64-linux-gnu
        /usr/aarch64-linux-gnu
)
# 在目标环境搜索 program
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# 在目标环境搜索库文件
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
# 在目标环境搜索头文件
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# 要运行的平台
list(APPEND VALID_PLATFORM qemu)
if (NOT DEFINED PLATFORM)
    set(PLATFORM qemu)
endif ()
message(STATUS "PLATFORM is: ${PLATFORM}")
# 如果不合法则报错
if (NOT PLATFORM IN_LIST VALID_PLATFORM)
    message(FATAL_ERROR "PLATFORM must be one of ${VALID_PLATFORM}")
endif ()

# 目标架构
list(APPEND VALID_CMAKE_SYSTEM_PROCESSOR x86_64 riscv64 aarch64)
# 默认构建 x86_64
if (NOT DEFINED CMAKE_SYSTEM_PROCESSOR)
    set(CMAKE_SYSTEM_PROCESSOR x86_64)
endif ()
message(STATUS "CMAKE_SYSTEM_PROCESSOR is: ${CMAKE_SYSTEM_PROCESSOR}")
# 如果不合法则报错
if (NOT CMAKE_SYSTEM_PROCESSOR IN_LIST VALID_CMAKE_SYSTEM_PROCESSOR)
    message(FATAL_ERROR "CMAKE_SYSTEM_PROCESSOR must be one of ${VALID_CMAKE_SYSTEM_PROCESSOR}")
endif ()

message(STATUS "CMAKE_TOOLCHAIN_FILE is: ${CMAKE_TOOLCHAIN_FILE}")
# 编译器只支持 gnu-gcc 或 clang
if (NOT ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang"))
    message(FATAL_ERROR "Only support gnu-gcc/clang")
endif ()

# qemu 运行依赖
if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    list(APPEND RUN_DEPENDS
            ovmf
    )
elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "riscv64")
    list(APPEND RUN_DEPENDS
            opensbi
    )
elseif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
    list(APPEND RUN_DEPENDS
            ovmf
    )
endif ()

# qemu 调试依赖
list(APPEND DEBUG_DEPENDS
        ${RUN_DEPENDS}
        gdbinit
)

# qemu gdb 调试端口
if (NOT DEFINED QEMU_GDB_PORT)
    set(QEMU_GDB_PORT tcp::1234)
endif ()

# qemu monitor 参数
if (NOT DEFINED QEMU_MONITOR_ARG)
    set(QEMU_MONITOR_ARG
            telnet::2333,server,nowait
    )
endif ()

# 生成项目配置头文件，传递给代码
configure_file(${CMAKE_SOURCE_DIR}/tools/project_config.h.in ${CMAKE_SOURCE_DIR}/src/project_config.h @ONLY)
