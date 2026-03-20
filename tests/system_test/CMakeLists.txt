# Copyright The SimpleKernel Contributors

PROJECT (system_test)

ENABLE_LANGUAGE (CXX)
ENABLE_LANGUAGE (ASM)

ADD_EXECUTABLE (
    ${PROJECT_NAME}
    main.cpp
    ctor_dtor_test.cpp
    spinlock_test.cpp
    mutex_test.cpp
    memory_test.cpp
    virtual_memory_test.cpp
    interrupt_test.cpp
    fifo_scheduler_test.cpp
    rr_scheduler_test.cpp
    cfs_scheduler_test.cpp
    idle_scheduler_test.cpp
    thread_group_test.cpp
    wait_test.cpp
    clone_test.cpp
    exit_test.cpp
    ramfs_test.cpp
    fatfs_test.cpp
    kernel_task_test.cpp
    user_task_test.cpp
    cross_core_test.cpp
    yield_test.cpp
    fork_test.cpp
    signal_test.cpp
    affinity_test.cpp
    tick_test.cpp
    zombie_reap_test.cpp
    stress_test.cpp
    balance_test.cpp
    ${CMAKE_SOURCE_DIR}/src/syscall.cpp
    ${CMAKE_SOURCE_DIR}/src/io_buffer.cpp)

TARGET_INCLUDE_DIRECTORIES (
    ${PROJECT_NAME}
    PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/include
            ${CMAKE_SOURCE_DIR}/src/arch/${CMAKE_SYSTEM_PROCESSOR})

TARGET_LINK_LIBRARIES (
    ${PROJECT_NAME}
    PRIVATE kernel_link_libraries
            libc
            libcxx
            arch
            memory
            device
            task
            filesystem)

# qemu 参数设置
LIST (APPEND ${PROJECT_NAME}_QEMU_BOOT_FLAGS
      "-D;${CMAKE_BINARY_DIR}/bin/${PROJECT_NAME}_qemu.log")
# 目标平台参数
IF(CMAKE_SYSTEM_PROCESSOR STREQUAL "riscv64")
    LIST (APPEND ${PROJECT_NAME}_QEMU_BOOT_FLAGS -bios
          ${u-boot_BINARY_DIR}/spl/u-boot-spl.bin -device
          loader,file=${u-boot_BINARY_DIR}/u-boot.itb,addr=0x80200000)
ELSEIF(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    LIST (
        APPEND
        ${PROJECT_NAME}_QEMU_BOOT_FLAGS
        -drive
        file=fat:rw:${CMAKE_BINARY_DIR}/bin,format=raw,media=disk
        -serial
        ${QEMU_NORMAL_WORLD_DEV_PATH}
        -serial
        ${QEMU_SECURE_WORLD_DEV_PATH}
        -bios
        ${arm-trusted-firmware_BINARY_DIR}/flash.bin
        -kernel
        $<TARGET_FILE:${PROJECT_NAME}>)
ENDIF()

# 添加 run 和 debug target
ADD_RUN_TARGET (
    NAME
    ${PROJECT_NAME}_
    TARGET
    ${PROJECT_NAME}
    DEPENDS
    u-boot
    $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},riscv64>:opensbi>
    $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},aarch64>:optee_os>
    $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},aarch64>:arm-trusted-firmware>
    QEMU_BOOT_FLAGS
    ${${PROJECT_NAME}_QEMU_BOOT_FLAGS})
