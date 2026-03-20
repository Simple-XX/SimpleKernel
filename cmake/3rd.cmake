# Copyright The SimpleKernel Contributors

# # https://github.com/abumq/easyloggingpp
# CPMAddPackage(
#   NAME easylogingpp
#   VERSION 9.97.0
#   GITHUB_REPOSITORY amrayn/easyloggingpp
#   OPTIONS
#   "build_static_lib ON"
#   "lib_utc_datetime ON"
# )

# https://github.com/rttrorg/rttr
# @bug 打开这个会导致编译参数中多出来几个
# CPMAddPackage(
#   NAME rttr # link against RTTR::Core_Lib
#   VERSION 0.9.6
#   GITHUB_REPOSITORY rttrorg/rttr
#   OPTIONS
#     "BUILD_RTTR_DYNAMIC Off"
#     "BUILD_UNIT_TESTS Off"
#     "BUILD_STATIC On"
#     "BUILD_PACKAGE Off"
#     "BUILD_WITH_RTTI On"
#     "BUILD_EXAMPLES Off"
#     "BUILD_DOCUMENTATION Off"
#     "BUILD_INSTALLER Off"
#     "USE_PCH Off"
#     "CUSTOM_DOXYGEN_STYLE Off"
# )

# https://github.com/TheLartians/Format.cmake
# CPMAddPackage(
#   NAME Format.cmake
#   GITHUB_REPOSITORY TheLartians/Format.cmake
#   VERSION 1.7.3
# )

# # https://github.com/freetype/freetype
# CPMAddPackage(
#   NAME freetype
#   GIT_REPOSITORY https://github.com/freetype/freetype.git
#   GIT_TAG VER-2-13-0
#   VERSION 2.13.0
# )
# if (freetype_ADDED)
#   add_library(Freetype::Freetype ALIAS freetype)
# endif()

# Pre-commit hooks
IF(NOT EXISTS ${CMAKE_SOURCE_DIR}/.git/hooks/pre-commit)
    EXECUTE_PROCESS (COMMAND pre-commit install)
ENDIF()

# https://github.com/google/googletest.git
IF(NOT TARGET gtest)
    ADD_SUBDIRECTORY (3rd/googletest)
    INCLUDE (GoogleTest)
ENDIF()

# https://github.com/MRNIU/bmalloc.git
ADD_SUBDIRECTORY (3rd/bmalloc)

# https://github.com/MRNIU/cpu_io.git
ADD_SUBDIRECTORY (3rd/cpu_io)

# https://github.com/MRNIU/MPMCQueue.git
ADD_SUBDIRECTORY (3rd/MPMCQueue)

# https://github.com/ETLCPP/etl.git
ADD_SUBDIRECTORY (3rd/etl)

# https://github.com/armink/EasyLogger.git
# ADD_SUBDIRECTORY (3rd/EasyLogger)

# https://github.com/abbrev/fatfs.git
# @todo 计划使用 c++ 重写
SET (fatfs_SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rd/fatfs)
SET (fatfs_BINARY_DIR ${CMAKE_BINARY_DIR}/3rd/fatfs)
ADD_LIBRARY (fatfs_lib INTERFACE)
TARGET_INCLUDE_DIRECTORIES (fatfs_lib INTERFACE ${fatfs_SOURCE_DIR}/source)
TARGET_SOURCES (fatfs_lib INTERFACE ${fatfs_SOURCE_DIR}/source/ff.c
                                    ${fatfs_SOURCE_DIR}/source/ffunicode.c)

IF(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "riscv64")
    # https://github.com/riscv-software-src/opensbi.git
    # 编译 opensbi
    SET (opensbi_SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rd/opensbi)
    SET (opensbi_BINARY_DIR ${CMAKE_BINARY_DIR}/3rd/opensbi)
    ADD_CUSTOM_TARGET (
        opensbi
        COMMENT "build opensbi..."
        # make 时编译
        ALL
        WORKING_DIRECTORY ${opensbi_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${opensbi_BINARY_DIR}
        COMMAND
            make PLATFORM_RISCV_XLEN=64 PLATFORM=generic FW_JUMP_ADDR=0x80210000
            FW_OPTIONS=0 CROSS_COMPILE=${TOOLCHAIN_PREFIX}
            O=${opensbi_BINARY_DIR} -j${CMAKE_BUILD_PARALLEL_LEVEL}
        COMMAND ln -s -f ${opensbi_SOURCE_DIR}/include ${opensbi_BINARY_DIR})

    # https://github.com/MRNIU/opensbi_interface.git
    ADD_SUBDIRECTORY (3rd/opensbi_interface)
ENDIF()

# https://github.com/u-boot/u-boot.git
SET (u-boot_SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rd/u-boot)
SET (u-boot_BINARY_DIR ${CMAKE_BINARY_DIR}/3rd/u-boot)
ADD_CUSTOM_TARGET (
    u-boot
    COMMENT "build u-boot..."
    # make 时编译
    ALL
    DEPENDS $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},riscv64>:opensbi>
    WORKING_DIRECTORY ${u-boot_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${u-boot_BINARY_DIR}
    COMMAND
        make O=${u-boot_BINARY_DIR}
        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},aarch64>:qemu_arm64_defconfig>
        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},riscv64>:qemu-riscv64_spl_defconfig>
        -j${CMAKE_BUILD_PARALLEL_LEVEL}
    COMMAND
        make CROSS_COMPILE=${TOOLCHAIN_PREFIX} O=${u-boot_BINARY_DIR}
        $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},riscv64>:OPENSBI=${opensbi_BINARY_DIR}/platform/generic/firmware/fw_dynamic.bin>
        -j${CMAKE_BUILD_PARALLEL_LEVEL})
SET_DIRECTORY_PROPERTIES (PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES
                                     ${u-boot_BINARY_DIR})

IF(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
    # https://github.com/OP-TEE/optee_os.git
    SET (optee_os_SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rd/optee/optee_os)
    SET (optee_os_BINARY_DIR ${CMAKE_BINARY_DIR}/3rd/optee/optee_os)
    ADD_CUSTOM_TARGET (
        optee_os
        COMMENT "build optee_os..."
        # make 时编译
        ALL
        WORKING_DIRECTORY ${optee_os_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${optee_os_BINARY_DIR}
        COMMAND
            make CFG_ARM64_core=y CFG_TEE_BENCHMARK=n CFG_TEE_CORE_LOG_LEVEL=3
            CROSS_COMPILE=${TOOLCHAIN_PREFIX}
            CROSS_COMPILE_core=${TOOLCHAIN_PREFIX}
            CROSS_COMPILE_ta_arm32=${TOOLCHAIN_PREFIX32}
            CROSS_COMPILE_ta_arm64=${TOOLCHAIN_PREFIX} DEBUG=$<CONFIG:Debug>
            O=${optee_os_BINARY_DIR} PLATFORM=vexpress-qemu_armv8a
            CFG_ARM_GICV3=y -j${CMAKE_BUILD_PARALLEL_LEVEL})
    SET_DIRECTORY_PROPERTIES (PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES
                                         ${optee_os_BINARY_DIR})

    # https://github.com/OP-TEE/optee_client.git
    ADD_SUBDIRECTORY (${CMAKE_SOURCE_DIR}/3rd/optee/optee_client)

    # https://github.com/ARM-software/arm-trusted-firmware
    # 编译 atf
    SET (arm-trusted-firmware_SOURCE_DIR
         ${CMAKE_SOURCE_DIR}/3rd/arm-trusted-firmware)
    SET (arm-trusted-firmware_BINARY_DIR
         ${CMAKE_BINARY_DIR}/3rd/arm-trusted-firmware)
    ADD_CUSTOM_TARGET (
        arm-trusted-firmware
        COMMENT "build arm-trusted-firmware..."
        # make 时编译
        ALL
        DEPENDS optee_os u-boot
        WORKING_DIRECTORY ${arm-trusted-firmware_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory
                ${arm-trusted-firmware_BINARY_DIR}
        COMMAND
            make DEBUG=$<CONFIG:Debug> CROSS_COMPILE=${TOOLCHAIN_PREFIX}
            PLAT=qemu BUILD_BASE=${arm-trusted-firmware_BINARY_DIR}
            BL32=${optee_os_BINARY_DIR}/core/tee-header_v2.bin
            BL32_EXTRA1=${optee_os_BINARY_DIR}/core/tee-pager_v2.bin
            BL32_EXTRA2=${optee_os_BINARY_DIR}/core/tee-pageable_v2.bin
            BL33=${u-boot_BINARY_DIR}/u-boot.bin BL32_RAM_LOCATION=tdram
            QEMU_USE_GIC_DRIVER=QEMU_GICV3 SPD=opteed all fip
            -j${CMAKE_BUILD_PARALLEL_LEVEL}
        COMMAND
            dd
            if=${arm-trusted-firmware_BINARY_DIR}/qemu/$<IF:$<CONFIG:Debug>,debug,release>/bl1.bin
            of=${arm-trusted-firmware_BINARY_DIR}/flash.bin bs=4096 conv=notrunc
        COMMAND
            dd
            if=${arm-trusted-firmware_BINARY_DIR}/qemu/$<IF:$<CONFIG:Debug>,debug,release>/fip.bin
            of=${arm-trusted-firmware_BINARY_DIR}/flash.bin seek=64 bs=4096
            conv=notrunc)
    SET_DIRECTORY_PROPERTIES (PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES
                                         ${arm-trusted-firmware_BINARY_DIR})
ENDIF()

# https://git.kernel.org/pub/scm/utils/dtc/dtc.git
SET (dtc_SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rd/dtc)
SET (dtc_BINARY_DIR ${CMAKE_BINARY_DIR}/3rd/dtc)
SET (dtc_CC ${CMAKE_C_COMPILER})
SET (dtc_AR ${CMAKE_AR})
# 编译 libfdt
IF(NOT EXISTS ${dtc_BINARY_DIR}/libfdt/libfdt.a)
    ADD_CUSTOM_TARGET (
        dtc
        COMMENT "build libdtc..."
        # make 时编译
        ALL
        WORKING_DIRECTORY ${dtc_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${dtc_BINARY_DIR}/libfdt
        COMMAND CC=${dtc_CC} AR=${dtc_AR} HOME=${dtc_BINARY_DIR} make
                libfdt/libfdt.a -j${CMAKE_BUILD_PARALLEL_LEVEL}
        COMMAND ${CMAKE_COMMAND} -E copy ${dtc_SOURCE_DIR}/libfdt/*.a
                ${dtc_SOURCE_DIR}/libfdt/*.h ${dtc_BINARY_DIR}/libfdt
        COMMAND make clean)
ELSE()
    ADD_CUSTOM_TARGET (
        dtc
        COMMENT "libdtc already exists, skipping..."
        # make 时编译
        ALL
        WORKING_DIRECTORY ${dtc_SOURCE_DIR})
ENDIF()
ADD_LIBRARY (dtc-lib INTERFACE)
ADD_DEPENDENCIES (dtc-lib dtc)
TARGET_INCLUDE_DIRECTORIES (dtc-lib INTERFACE ${dtc_BINARY_DIR}/libfdt)
TARGET_LINK_LIBRARIES (dtc-lib INTERFACE ${dtc_BINARY_DIR}/libfdt/libfdt.a)

# doxygen
FIND_PACKAGE (Doxygen REQUIRED dot)

IF(CMAKE_SYSTEM_PROCESSOR STREQUAL CMAKE_HOST_SYSTEM_PROCESSOR)
    # genhtml 生成测试覆盖率报告网页
    FIND_PROGRAM (GENHTML_EXE genhtml)
    # lcov 生成测试覆盖率报告
    FIND_PROGRAM (LCOV_EXE lcov)
ENDIF()
