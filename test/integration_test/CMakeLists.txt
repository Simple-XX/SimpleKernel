
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# CMakeLists.txt for Simple-XX/SimpleKernel.

# 设置项目名与版本
project(
        integration-test
        VERSION 0.0.1
)

add_executable(integration_test
        example.cpp
)

target_compile_options(integration_test PRIVATE
        ${DEFAULT_TEST_COMPILE_OPTIONS}
)

target_link_options(integration_test PRIVATE
        ${DEFAULT_TEST_LINK_OPTIONS}
)

target_link_libraries(integration_test PRIVATE
        ${DEFAULT_TEST_LINK_LIB}
)

gtest_discover_tests(integration_test)
