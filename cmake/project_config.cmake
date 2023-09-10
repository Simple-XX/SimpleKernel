
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

# 设置清理目标 在 make clean 时删除文件夹
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
    # 删除 .gdbinit
    ${CMAKE_SOURCE_DIR}/.gdbinit
)

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
list(APPEND VALID_TARGET_ARCH x86_64 riscv64 aarch64)
# 默认构建 x86_64
if (NOT DEFINED TARGET_ARCH)
    set(TARGET_ARCH x86_64)
endif ()
message(STATUS "TARGET_ARCH is: ${TARGET_ARCH}")
# 如果不合法则报错
if (NOT TARGET_ARCH IN_LIST VALID_TARGET_ARCH)
    message(FATAL_ERROR "TARGET_ARCH must be one of ${VALID_TARGET_ARCH}")
endif ()

message(STATUS "CMAKE_TOOLCHAIN_FILE is: ${CMAKE_TOOLCHAIN_FILE}")
# 编译器只支持 gnu-gcc 或 clang
if (NOT ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang"))
    message(FATAL_ERROR "Only support gnu-gcc/clang")
endif ()

# qemu 运行依赖
if (${TARGET_ARCH} STREQUAL "x86_64")
    list(APPEND RUN_DEPENDS
        ovmf
    )
elseif (${TARGET_ARCH} STREQUAL "riscv64")
    list(APPEND RUN_DEPENDS
        opensbi
    )
elseif (${TARGET_ARCH} STREQUAL "aarch64")
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
