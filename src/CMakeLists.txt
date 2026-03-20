# Copyright The SimpleKernel Contributors

PROJECT (SimpleKernel)

ENABLE_LANGUAGE (ASM)
ENABLE_LANGUAGE (C)
ENABLE_LANGUAGE (CXX)

ADD_SUBDIRECTORY (libc)
ADD_SUBDIRECTORY (libcxx)
ADD_SUBDIRECTORY (arch)
ADD_SUBDIRECTORY (memory)
ADD_SUBDIRECTORY (device)
ADD_SUBDIRECTORY (task)
ADD_SUBDIRECTORY (filesystem)

ADD_EXECUTABLE (${PROJECT_NAME} main.cpp io_buffer.cpp syscall.cpp)

# 添加头文件
TARGET_INCLUDE_DIRECTORIES (${PROJECT_NAME} PRIVATE ./ include)

# 添加要链接的库
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

SET_TARGET_PROPERTIES (${PROJECT_NAME} PROPERTIES OUTPUT_NAME
                                                  ${KERNEL_ELF_OUTPUT_NAME})

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
    TARGET
    ${PROJECT_NAME}
    DEPENDS
    u-boot
    $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},riscv64>:opensbi>
    $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},aarch64>:optee_os>
    $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},aarch64>:arm-trusted-firmware>
    QEMU_BOOT_FLAGS
    ${${PROJECT_NAME}_QEMU_BOOT_FLAGS})
