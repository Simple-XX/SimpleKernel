# Copyright The SimpleKernel Contributors

# 设置项目名与版本
PROJECT (unit-test)

ENABLE_LANGUAGE (CXX)

ADD_EXECUTABLE (
    ${PROJECT_NAME}
    mocks/arch.cpp
    mocks/test_environment_state.cpp
    mocks/io_buffer_mock.cpp
    sk_libc_test.cpp
    sk_ctype_test.cpp
    sk_string_test.cpp
    kernel_elf_test.cpp
    kernel_fdt_test.cpp
    spinlock_test.cpp
    virtual_memory_test.cpp
    task/fixtures/task_test_harness.cpp
    vfs_test.cpp
    ramfs_test.cpp
    ${CMAKE_SOURCE_DIR}/src/memory/virtual_memory.cpp
    ${CMAKE_SOURCE_DIR}/src/syscall.cpp
    ${CMAKE_SOURCE_DIR}/src/task/block.cpp
    ${CMAKE_SOURCE_DIR}/src/task/clone.cpp
    ${CMAKE_SOURCE_DIR}/src/task/exit.cpp
    ${CMAKE_SOURCE_DIR}/src/task/schedule.cpp
    ${CMAKE_SOURCE_DIR}/src/task/sleep.cpp
    ${CMAKE_SOURCE_DIR}/src/task/task_control_block.cpp
    ${CMAKE_SOURCE_DIR}/src/task/task_manager.cpp
    ${CMAKE_SOURCE_DIR}/src/task/tick_update.cpp
    ${CMAKE_SOURCE_DIR}/src/task/wait.cpp
    virtio_driver_test.cpp
    dma_region_test.cpp
    balance_test.cpp)

TARGET_COMPILE_DEFINITIONS (
    ${PROJECT_NAME}
    PRIVATE -DUNIT_TEST
            -DSIMPLEKERNEL_SRC_LIBCXX_INCLUDE_KSTD_NEW_
            -DSIMPLEKERNEL_MAX_CORE_COUNT=4
            -DSIMPLEKERNEL_PER_CPU_ALIGN_SIZE=4096
            -DSIMPLEKERNEL_TICK=1000
            -DSIMPLEKERNEL_MIN_LOG_LEVEL=0
            -DSIMPLEKERNEL_EARLY_CONSOLE_BASE=${SIMPLEKERNEL_EARLY_CONSOLE_BASE}
)

TARGET_INCLUDE_DIRECTORIES (
    ${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/mocks
            ${CMAKE_CURRENT_SOURCE_DIR}/task/fixtures
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/src/include
            ${CMAKE_SOURCE_DIR}/src/arch
            ${CMAKE_SOURCE_DIR}/src/libc/include
            ${CMAKE_SOURCE_DIR}/src/libcxx/include
            ${CMAKE_SOURCE_DIR}/src/memory/include
            ${CMAKE_SOURCE_DIR}/src/task/include
            ${CMAKE_SOURCE_DIR}/src/filesystem/include
            ${CMAKE_SOURCE_DIR}/src/filesystem/vfs/include
            ${CMAKE_SOURCE_DIR}/src/filesystem/ramfs/include
            ${CMAKE_SOURCE_DIR}/src/device
            ${CMAKE_SOURCE_DIR}/src/device/include
            ${CMAKE_SOURCE_DIR}/src/device/ns16550a
            ${CMAKE_SOURCE_DIR}/src/device/pl011
            ${CMAKE_SOURCE_DIR}/src/device/acpi
            ${CMAKE_SOURCE_DIR}/src/device/virtio)

TARGET_COMPILE_OPTIONS (${PROJECT_NAME} PRIVATE ${DEFAULT_TEST_COMPILE_OPTIONS})

TARGET_LINK_OPTIONS (${PROJECT_NAME} PRIVATE ${DEFAULT_TEST_LINK_OPTIONS})

TARGET_LINK_LIBRARIES (
    ${PROJECT_NAME}
    PRIVATE ${DEFAULT_TEST_LINK_LIB}
            dtc-lib
            bmalloc
            MPMCQueue
            cpu_io
            device
            task
            filesystem
            fatfs_lib
            3rd_compile_definitions
            etl::etl)

GTEST_DISCOVER_TESTS (${PROJECT_NAME})
