
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

# 设置项目名与版本
project(
        gnu-efi-test
        VERSION 0.0.1
)

enable_language(CXX)

add_executable(${PROJECT_NAME}
        main.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/kernel/include
        ${CMAKE_SOURCE_DIR}/src/kernel/arch
        ${CMAKE_SOURCE_DIR}/src/kernel/arch/${CMAKE_SYSTEM_PROCESSOR}/include
)

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
        DEPENDS boot ${PROJECT_NAME}
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        TARGET ${CMAKE_SYSTEM_PROCESSOR}
        BOOT ${boot_BINARY_DIR}/${BOOT_EFI_OUTPUT_NAME}
        KERNEL $<TARGET_FILE:${PROJECT_NAME}>
        QEMU_FLAGS ${QEMU_FLAGS}
)
