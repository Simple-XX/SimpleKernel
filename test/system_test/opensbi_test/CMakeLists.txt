
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

add_executable(${PROJECT_NAME}_kernel
        boot.cpp
        ${CMAKE_SOURCE_DIR}/src/kernel/arch/${CMAKE_SYSTEM_PROCESSOR}/boot.S
)

add_header_libcxx(${PROJECT_NAME}_kernel)
add_header_3rd(${PROJECT_NAME}_kernel)

target_compile_options(${PROJECT_NAME}_kernel PRIVATE
        ${DEFAULT_KERNEL_COMPILE_OPTIONS}
)

target_link_options(${PROJECT_NAME}_kernel PRIVATE
        ${DEFAULT_KERNEL_LINK_OPTIONS}
)

# 添加要链接的库
target_link_libraries(${PROJECT_NAME}_kernel PRIVATE
        ${DEFAULT_KERNEL_LINK_LIB}
        libcxx
)

add_dependencies(${PROJECT_NAME}_kernel
        ${COMPILE_DEPENDS}
)

set_target_properties(${PROJECT_NAME}_kernel PROPERTIES PREFIX "")
set_target_properties(${PROJECT_NAME}_kernel PROPERTIES OUTPUT_NAME ${KERNEL_ELF_OUTPUT_NAME})

readelf_a(${PROJECT_NAME}_kernel)
objdump_D(${PROJECT_NAME}_kernel)

# 添加 run 和 debug target
add_run_target(NAME ${PROJECT_NAME}
        DEPENDS ${PROJECT_NAME}_kernel ${RUN_DEPENDS} ${DEBUG_DEPENDS}
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        TARGET ${CMAKE_SYSTEM_PROCESSOR}
        KERNEL $<TARGET_FILE:${PROJECT_NAME}_kernel>
        QEMU_FLAGS
        -serial stdio -monitor telnet::2333,server,nowait
        -machine virt -nographic
        -bios ${opensbi_BINARY_DIR}/platform/generic/firmware/fw_jump.elf
        -kernel $<TARGET_FILE:${PROJECT_NAME}_kernel>
)
