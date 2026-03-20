# Copyright The SimpleKernel Contributors

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

IF(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "riscv64")
    ADD_SUBDIRECTORY (opensbi_test)
ENDIF()
