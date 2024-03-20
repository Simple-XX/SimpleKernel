
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

add_header_libc(${PROJECT_NAME})
add_header_libcxx(${PROJECT_NAME})
add_header_arch(${PROJECT_NAME})
add_header_kernel(${PROJECT_NAME})

target_compile_options(${PROJECT_NAME} PRIVATE
        ${DEFAULT_KERNEL_COMPILE_OPTIONS}
)

target_link_options(${PROJECT_NAME} PRIVATE
        ${DEFAULT_KERNEL_LINK_OPTIONS}
)

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

# readelf -a
readelf_a(${PROJECT_NAME})

# objdump -D
objdump_D(${PROJECT_NAME})

# 添加 run 和 debug target
add_run_target(NAME ${PROJECT_NAME}
        DEPENDS boot ${PROJECT_NAME} ${RUN_DEPENDS} ${DEBUG_DEPENDS}
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        TARGET ${TARGET_ARCH}
        BOOT ${boot_BINARY_DIR}/${BOOT_EFI_OUTPUT_NAME}
        KERNEL $<TARGET_FILE:${PROJECT_NAME}>
        QEMU_FLAGS ${QEMU_FLAGS}
)
