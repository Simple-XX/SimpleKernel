
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

add_executable(${PROJECT_NAME}_boot
        boot.cpp
        ${CMAKE_SOURCE_DIR}/src/boot/load_elf.cpp
        ${CMAKE_SOURCE_DIR}/src/boot/ostream.cpp
        ${CMAKE_SOURCE_DIR}/src/boot/graphics.cpp
        ${CMAKE_SOURCE_DIR}/src/boot/memory.cpp
)

add_header_project(${PROJECT_NAME}_boot)
add_header_boot(${PROJECT_NAME}_boot)
add_header_3rd(${PROJECT_NAME}_boot)

target_compile_options(${PROJECT_NAME}_boot PRIVATE
        ${DEFAULT_BOOT_COMPILE_OPTIONS}
)

target_link_options(${PROJECT_NAME}_boot PRIVATE
        ${DEFAULT_BOOT_LINK_OPTIONS}
)

target_link_libraries(${PROJECT_NAME}_boot PRIVATE
        ${DEFAULT_BOOT_LINK_LIB}
)

add_dependencies(${PROJECT_NAME}_boot
        ${COMPILE_DEPENDS}
)

set_target_properties(${PROJECT_NAME}_boot PROPERTIES PREFIX "")
set_target_properties(${PROJECT_NAME}_boot PROPERTIES OUTPUT_NAME ${BOOT_ELF_OUTPUT_NAME})

# readelf -a
readelf_a(${PROJECT_NAME}_boot)

# objdump -D
objdump_D(${PROJECT_NAME}_boot)

# 生成 efi
elf2efi(${PROJECT_NAME}_boot ${BOOT_EFI_OUTPUT_NAME})

add_executable(${PROJECT_NAME}_kernel
        main.cpp
)

add_header_kernel(${PROJECT_NAME}_kernel)
add_header_arch(${PROJECT_NAME}_kernel)

target_compile_options(${PROJECT_NAME}_kernel PRIVATE
        ${DEFAULT_KERNEL_COMPILE_OPTIONS}
)

target_link_options(${PROJECT_NAME}_kernel PRIVATE
        ${DEFAULT_KERNEL_LINK_OPTIONS}
)

set_target_properties(${PROJECT_NAME}_kernel PROPERTIES PREFIX "")
set_target_properties(${PROJECT_NAME}_kernel PROPERTIES OUTPUT_NAME ${KERNEL_ELF_OUTPUT_NAME})

# readelf -a
readelf_a(${PROJECT_NAME}_kernel)

# objdump -D
objdump_D(${PROJECT_NAME}_kernel)

# 添加 run 和 debug target
add_run_target(NAME ${PROJECT_NAME}
        DEPENDS ${PROJECT_NAME}_boot ${PROJECT_NAME}_kernel ${RUN_DEPENDS} ${DEBUG_DEPENDS}
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        TARGET ${TARGET_ARCH}
        BOOT ${BOOT_EFI_OUTPUT_NAME}
        KERNEL $<TARGET_FILE:${PROJECT_NAME}_kernel>
        QEMU_FLAGS ${QEMU_FLAGS}
)
