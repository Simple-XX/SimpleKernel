
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

# 设置项目名与版本
project(
        test
        VERSION 0.0.1
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/system_test)

# 仅在 host 环境下进行 ut 和 it
if (TARGET_ARCH STREQUAL CMAKE_HOST_SYSTEM_PROCESSOR)
    enable_testing()
    include(GoogleTest)

    list(APPEND DEFAULT_TEST_COMPILE_OPTIONS
            --coverage
    )

    list(APPEND DEFAULT_TEST_LINK_OPTIONS
            --coverage
            -fsanitize=leak
            -fsanitize=address
            -fno-omit-frame-pointer
    )

    list(APPEND DEFAULT_TEST_LINK_LIB
            arch
            driver
            libc
            libcxx
            gtest_main
    )

    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/unit_test)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/integration_test)

    add_coverage_target(
            DEPENDS unit_test integration_test
            SOURCE_DIR ${CMAKE_SOURCE_DIR}
            BINARY_DIR ${CMAKE_BINARY_DIR}
            EXCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rd/*
    )
endif ()
