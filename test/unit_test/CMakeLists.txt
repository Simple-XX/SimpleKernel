
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

# 设置项目名与版本
project(
        unit-test
        VERSION 0.0.1
)

enable_language(CXX)

add_executable(${PROJECT_NAME}
        riscv64_cpu_test.cpp
        x86_64_cpu_test.cpp
        aarch64_cpu_test.cpp
        kernel_elf_test.cpp
        kernel_fdt_test.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src/kernel/include
        ${CMAKE_SOURCE_DIR}/src/kernel/arch
)

target_compile_options(${PROJECT_NAME} PRIVATE
        ${DEFAULT_TEST_COMPILE_OPTIONS}
)

target_link_options(${PROJECT_NAME} PRIVATE
        ${DEFAULT_TEST_LINK_OPTIONS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        ${DEFAULT_TEST_LINK_LIB}
        ${dtc_BINARY_DIR}/libfdt/libfdt.a
)

gtest_discover_tests(${PROJECT_NAME})
