
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

# 设置项目名与版本
project(
        opensbi-test
        VERSION 0.0.1
)

enable_language(ASM)
enable_language(CXX)

add_executable(${PROJECT_NAME}
        main.cpp
        ${CMAKE_SOURCE_DIR}/src/kernel/arch/${CMAKE_SYSTEM_PROCESSOR}/boot.S
)

add_header_libc(${PROJECT_NAME})
add_header_libcxx(${PROJECT_NAME})
add_header_arch(${PROJECT_NAME})
add_header_3rd(${PROJECT_NAME})

target_compile_options(${PROJECT_NAME} PRIVATE
        ${DEFAULT_KERNEL_COMPILE_OPTIONS}
)

target_link_options(${PROJECT_NAME} PRIVATE
        ${DEFAULT_KERNEL_LINK_OPTIONS}
)

# 添加要链接的库
target_link_libraries(${PROJECT_NAME} PRIVATE
        ${DEFAULT_KERNEL_LINK_LIB}
        libc
        libcxx
)

add_dependencies(${PROJECT_NAME}
        ${COMPILE_DEPENDS}
)

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${KERNEL_ELF_OUTPUT_NAME})

# 获取目标文件信息
objdump_readelf_nm(${PROJECT_NAME})

# 添加 run 和 debug target
add_run_target(NAME ${PROJECT_NAME}_
        DEPENDS ${PROJECT_NAME} ${RUN_DEPENDS} ${DEBUG_DEPENDS}
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        TARGET ${CMAKE_SYSTEM_PROCESSOR}
        KERNEL $<TARGET_FILE:${PROJECT_NAME}>
        QEMU_FLAGS
        -serial stdio -monitor telnet::2333,server,nowait
        -machine virt -nographic
        -bios ${opensbi_BINARY_DIR}/platform/generic/firmware/fw_jump.elf
        -kernel $<TARGET_FILE:${PROJECT_NAME}>
)
