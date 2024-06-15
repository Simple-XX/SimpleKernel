
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# 3rd.cmake for Simple-XX/SimpleKernel.
# 依赖管理

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

# https://github.com/google/googletest.git
if (NOT TARGET gtest)
        add_subdirectory(3rd/googletest)
        include(GoogleTest)
endif()

# https://github.com/gdbinit/Gdbinit.git
set(gdbinit_SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rd/gdbinit)
set(gdbinit_BINARY_DIR ${CMAKE_BINARY_DIR}/3rd/gdbinit)
add_custom_target(gdbinit
        COMMENT "Generate gdbinit ..."
        WORKING_DIRECTORY ${gdbinit_SOURCE_DIR}
        # 复制到根目录下并重命名
        COMMAND
        ${CMAKE_COMMAND}
        -E
        copy
        ${gdbinit_SOURCE_DIR}/gdbinit
        ${CMAKE_SOURCE_DIR}/.gdbinit
        COMMAND
        echo "target remote ${QEMU_GDB_PORT}" >> ${CMAKE_SOURCE_DIR}/.gdbinit
        COMMAND
        echo "add-symbol-file ${kernel_BINARY_DIR}/${KERNEL_ELF_OUTPUT_NAME}" >> ${CMAKE_SOURCE_DIR}/.gdbinit
        COMMAND
        echo "add-symbol-file ${boot_BINARY_DIR}/${BOOT_ELF_OUTPUT_NAME}" >> ${CMAKE_SOURCE_DIR}/.gdbinit
)

# https://github.com/MRNIU/printf_bare_metal.git
add_subdirectory(3rd/printf_bare_metal)

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "riscv64")
        # https://github.com/riscv-software-src/opensbi.git
        # 编译 opensbi
        set(opensbi_SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rd/opensbi)
        set(opensbi_BINARY_DIR ${CMAKE_BINARY_DIR}/3rd/opensbi)
        add_custom_target(opensbi
                COMMENT "build opensbi..."
                # make 时编译
                ALL
                WORKING_DIRECTORY ${opensbi_SOURCE_DIR}
                COMMAND
                ${CMAKE_COMMAND}
                -E
                make_directory
                ${opensbi_BINARY_DIR}
                COMMAND
                make
                CROSS_COMPILE=${TOOLCHAIN_PREFIX}
                FW_JUMP=y
                FW_JUMP_ADDR=0x80210000
                PLATFORM_RISCV_XLEN=64
                PLATFORM=generic
                O=${opensbi_BINARY_DIR}
                COMMAND
                ${CMAKE_COMMAND}
                -E
                copy_directory
                ${opensbi_SOURCE_DIR}/include
                ${opensbi_BINARY_DIR}/include
        )

        # https://github.com/MRNIU/opensbi_interface.git
        add_subdirectory(3rd/opensbi_interface)
endif ()

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "riscv64" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
        # https://git.kernel.org/pub/scm/utils/dtc/dtc.git
        set(dtc_SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rd/dtc)
        set(dtc_BINARY_DIR ${CMAKE_BINARY_DIR}/3rd/dtc)
        set(dtc_CC ${CMAKE_C_COMPILER})
        set(dtc_AR ${CMAKE_AR})
        # 编译 libfdt
        add_custom_target(dtc
                COMMENT "build libdtc..."
                # make 时编译
                ALL
                WORKING_DIRECTORY ${dtc_SOURCE_DIR}
                COMMAND
                ${CMAKE_COMMAND}
                -E
                make_directory
                ${dtc_BINARY_DIR}/libfdt
                COMMAND
                CC=${dtc_CC}
                AR=${dtc_AR}
                make libfdt/libfdt.a
                COMMAND
                ${CMAKE_COMMAND}
                -E
                copy
                ${dtc_SOURCE_DIR}/libfdt/*.a
                ${dtc_SOURCE_DIR}/libfdt/*.h
                ${dtc_BINARY_DIR}/libfdt
        )
endif ()

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
        # https://github.com/ncroxon/gnu-efi.git
        set(gnu-efi_SOURCE_DIR ${CMAKE_SOURCE_DIR}/3rd/gnu-efi)
        set(gnu-efi_BINARY_DIR ${CMAKE_BINARY_DIR}/3rd/gnu-efi)
        if (CMAKE_SYSTEM_PROCESSOR STREQUAL CMAKE_HOST_SYSTEM_PROCESSOR)
                set(CC_ ${CMAKE_C_COMPILER})
                set(AR_ ${CMAKE_AR})
        elseif (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "aarch64" AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
                set(CROSS_COMPILE_ x86_64-linux-gnu-)
        endif ()
        # 编译 gnu-efi
        add_custom_target(gnu-efi
                COMMENT "build gnu-efi..."
                # make 时编译
                ALL
                WORKING_DIRECTORY ${gnu-efi_SOURCE_DIR}
                COMMAND
                ${CMAKE_COMMAND}
                -E
                make_directory
                ${gnu-efi_BINARY_DIR}
                COMMAND
                # @note 仅支持 gcc
                make lib gnuefi inc
                CROSS_COMPILE=${CROSS_COMPILE_}
                ARCH=${CMAKE_SYSTEM_PROCESSOR}
                OBJDIR=${gnu-efi_BINARY_DIR}
                V=1
                COMMAND
                ${CMAKE_COMMAND}
                -E
                copy_directory
                ${gnu-efi_SOURCE_DIR}/inc
                ${gnu-efi_BINARY_DIR}/inc
        )

    # ovmf
    # @todo 使用互联网连接或从 edk2 编译
    # https://efi.akeo.ie/QEMU_EFI/QEMU_EFI-AA64.zip
        set(ovmf_SOURCE_DIR ${CMAKE_SOURCE_DIR}/tools/ovmf)
        set(ovmf_BINARY_DIR ${CMAKE_BINARY_DIR}/3rd/ovmf)
        add_custom_target(ovmf
                COMMENT "build ovmf ..."
                # make 时编译
                ALL
                WORKING_DIRECTORY ${ovmf_SOURCE_DIR}
                COMMAND
                ${CMAKE_COMMAND}
                -E
                make_directory
                ${ovmf_BINARY_DIR}
                COMMAND
                ${CMAKE_COMMAND}
                -E
                copy
                ${ovmf_SOURCE_DIR}/*
                ${ovmf_BINARY_DIR}
        )
endif ()

# gdb
find_program(GDB_EXE gdb)
if (NOT GDB_EXE)
    message(FATAL_ERROR "gdb not found.\n"
            "Following https://www.sourceware.org/gdb/ to install.")
endif ()

# qemu
find_program(QEMU_EXE qemu-system-${CMAKE_SYSTEM_PROCESSOR})
if (NOT QEMU_EXE)
    message(FATAL_ERROR "qemu-system-${CMAKE_SYSTEM_PROCESSOR} not found.\n"
            "Following https://www.qemu.org/ to install.")
endif ()

# doxygen
find_package(Doxygen
        REQUIRED dot)
if (NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Doxygen not found.\n"
            "Following https://www.doxygen.nl/index.html to install.")
endif ()

# cppcheck
find_program(CPPCHECK_EXE NAMES cppcheck)
if (NOT CPPCHECK_EXE)
    message(FATAL_ERROR "cppcheck not found.\n"
            "Following https://cppcheck.sourceforge.io to install.")
endif ()
add_custom_target(cppcheck
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Run cppcheck on ${CMAKE_BINARY_DIR}/compile_commands.json ..."
        COMMAND
        ${CPPCHECK_EXE}
        --enable=all
        --project=${CMAKE_BINARY_DIR}/compile_commands.json
        --suppress-xml=${CMAKE_SOURCE_DIR}/tools/cppcheck-suppressions.xml
        --output-file=${CMAKE_BINARY_DIR}/cppcheck_report.log
)

# 获取全部源文件
file(GLOB_RECURSE ALL_SOURCE_FILES
        ${CMAKE_SOURCE_DIR}/src/*.h
        ${CMAKE_SOURCE_DIR}/src/*.hpp
        ${CMAKE_SOURCE_DIR}/src/*.c
        ${CMAKE_SOURCE_DIR}/src/*.cpp
        ${CMAKE_SOURCE_DIR}/test/*.h
        ${CMAKE_SOURCE_DIR}/test/*.hpp
        ${CMAKE_SOURCE_DIR}/test/*.c
        ${CMAKE_SOURCE_DIR}/test/*.cpp
)

# clang-tidy
find_program(CLANG_TIDY_EXE NAMES clang-tidy)
if (NOT CLANG_TIDY_EXE)
    message(FATAL_ERROR "clang-tidy not found.\n"
            "Following https://clang.llvm.org/extra/clang-tidy to install.")
endif ()
add_custom_target(clang-tidy
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Run clang-tidy on ${ALL_SOURCE_FILES} ..."
        COMMAND
        ${CLANG_TIDY_EXE}
        --config-file=${CMAKE_SOURCE_DIR}/.clang-tidy
        -p=${CMAKE_BINARY_DIR}
        ${ALL_SOURCE_FILES}
        > ${CMAKE_BINARY_DIR}/clang_tidy_report.log 2>&1
)

# clang-format
find_program(CLANG_FORMAT_EXE NAMES clang-format)
if (NOT CLANG_FORMAT_EXE)
    message(FATAL_ERROR "clang-format not found.\n"
            "Following https://clang.llvm.org/docs/ClangFormat.html to install.")
endif ()
add_custom_target(clang-format
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Run clang-format on ${ALL_SOURCE_FILES} ..."
        COMMAND ${CLANG_FORMAT_EXE} -i -style=file ${ALL_SOURCE_FILES}
)

if (CMAKE_SYSTEM_PROCESSOR STREQUAL CMAKE_HOST_SYSTEM_PROCESSOR)
    # genhtml 生成测试覆盖率报告网页
    find_program(GENHTML_EXE genhtml)
    if (NOT GENHTML_EXE)
        message(FATAL_ERROR "genhtml not found.\n"
                "Following https://github.com/linux-test-project/lcov to install.")
    endif ()

    # lcov 生成测试覆盖率报告
    find_program(LCOV_EXE lcov)
    if (NOT LCOV_EXE)
        message(FATAL_ERROR "lcov not found.\n"
                "Following https://github.com/linux-test-project/lcov to install.")
    endif ()
endif ()
