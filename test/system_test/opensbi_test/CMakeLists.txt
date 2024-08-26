
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

# 添加要链接的库
target_link_libraries(${PROJECT_NAME} PRIVATE
        kernel_link_libraries
        libc
        libcxx
)

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${KERNEL_ELF_OUTPUT_NAME})

# 获取目标文件信息
objdump_readelf_nm(${PROJECT_NAME})

# 添加 run 和 debug target
add_run_target(NAME ${PROJECT_NAME}_
        DEPENDS ${PROJECT_NAME}
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        TARGET ${CMAKE_SYSTEM_PROCESSOR}
        BOOT ${boot_BINARY_DIR}/${BOOT_EFI_OUTPUT_NAME}
        KERNEL $<TARGET_FILE:${PROJECT_NAME}>
        QEMU_FLAGS
        -serial stdio -monitor telnet::2333,server,nowait
        -machine virt -nographic
        -bios ${opensbi_BINARY_DIR}/platform/generic/firmware/fw_jump.elf
        -kernel $<TARGET_FILE:${PROJECT_NAME}>
)
