
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# 3rd.cmake for Simple-XX/SimpleKernel.
# 依赖管理

# 设置依赖下载路径
set(CPM_SOURCE_CACHE ${CMAKE_SOURCE_DIR}/3rd)
# 优先使用本地文件
set(CPM_USE_LOCAL_PACKAGES True)
# https://github.com/cpm-cmake/CPM.cmake
# -------- get_cpm.cmake --------
set(CPM_DOWNLOAD_VERSION 0.38.2)

if (CPM_SOURCE_CACHE)
    set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
elseif (DEFINED ENV{CPM_SOURCE_CACHE})
    set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
else ()
    set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif ()

# Expand relative path. This is important if the provided path contains a tilde (~)
get_filename_component(CPM_DOWNLOAD_LOCATION ${CPM_DOWNLOAD_LOCATION} ABSOLUTE)

function(download_cpm)
    message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
    file(DOWNLOAD
            https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
            ${CPM_DOWNLOAD_LOCATION}
    )
endfunction()

if (NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
    download_cpm()
else ()
    # resume download if it previously failed
    file(READ ${CPM_DOWNLOAD_LOCATION} check)
    if ("${check}" STREQUAL "")
        download_cpm()
    endif ()
    unset(check)
endif ()

include(${CPM_DOWNLOAD_LOCATION})
# -------- get_cpm.cmake --------

# https://github.com/google/googletest
CPMAddPackage(
        NAME googletest
        GITHUB_REPOSITORY google/googletest
        GIT_TAG v1.13.0
        VERSION 1.13.0
        OPTIONS
        "INSTALL_GTEST OFF"
        "gtest_force_shared_crt ON"
)

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

if (${TARGET_ARCH} STREQUAL "riscv64")
    # https://github.com/riscv-software-src/opensbi
    CPMAddPackage(
            NAME opensbi
            GIT_REPOSITORY https://github.com/riscv-software-src/opensbi.git
            GIT_TAG v1.4
            VERSION 1.4
            DOWNLOAD_ONLY True
    )
    if (opensbi_ADDED)
        # 编译 opensbi
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
                FW_JUMP_ADDR=0x80200000
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
    endif ()
endif ()

if (${TARGET_ARCH} STREQUAL "x86_64" OR ${TARGET_ARCH} STREQUAL "aarch64")
    # https://sourceforge.net/projects/gnu-efi/
    CPMAddPackage(
            NAME gnu-efi
            URL "https://sourceforge.net/projects/gnu-efi/files/gnu-efi-3.0.17.tar.bz2"
            VERSION 3.0.17
            DOWNLOAD_ONLY True
    )
    if (gnu-efi_ADDED)
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
                make lib gnuefi inc
                # @note 仅支持 gcc
                CC=${CMAKE_C_COMPILER}
                AR=${CMAKE_AR}
                ARCH=${TARGET_ARCH}
                OBJDIR=${gnu-efi_BINARY_DIR}
                COMMAND
                ${CMAKE_COMMAND}
                -E
                copy_directory
                ${gnu-efi_SOURCE_DIR}/inc
                ${gnu-efi_BINARY_DIR}/inc
        )
    endif ()

    # ovmf
    # @todo 使用互联网连接或从 edk2 编译
    CPMAddPackage(
            NAME ovmf
            SOURCE_DIR ${PROJECT_SOURCE_DIR}/tools/ovmf
    )
    if (ovmf_ADDED)
        add_custom_target(ovmf
                COMMENT "build ovmf ..."
                # make 时编译
                ALL
                WORKING_DIRECTORY ${ovmf_SOURCE_DIR}
                COMMAND
                ${CMAKE_COMMAND}
                -E
                copy
                ${ovmf_SOURCE_DIR}/*
                ${ovmf_BINARY_DIR}
        )
    endif ()

    # # https://github.com/tianocore/edk2
    # # @todo 下载下来的文件为 makefile 形式，需要自己编译
    # CPMAddPackage(
    #   NAME edk2
    #   GIT_REPOSITORY https://github.com/tianocore/edk2.git
    #   GIT_TAG edk2-stable202305
    #   DOWNLOAD_ONLY True
    # )
endif ()

# https://github.com/gdbinit/Gdbinit
CPMAddPackage(
        NAME gdbinit
        GIT_REPOSITORY https://github.com/gdbinit/Gdbinit.git
        GIT_TAG e5138c24226bdd05360ca41743d8315a9e366c40
        DOWNLOAD_ONLY True
)
if (gdbinit_ADDED)
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
endif ()

# https://github.com/libcxxrt/libcxxrt
CPMAddPackage(
        NAME libcxxrt
        GIT_REPOSITORY https://github.com/libcxxrt/libcxxrt
        GIT_TAG a0f7f5c139a7daf71de0de201b6c405d852b1dc1
)
if (libcxxrt_ADDED)
    target_compile_options(cxxrt-static PRIVATE
            -fPIC
    )
endif ()

# https://github.com/MRNIU/opensbi_interface
CPMAddPackage(
        NAME opensbi_interface
        GIT_REPOSITORY https://github.com/MRNIU/opensbi_interface
        GIT_TAG v1.0.6
)

# https://github.com/cpm-cmake/CPMLicenses.cmake
# 保持在 CPMAddPackage 的最后
CPMAddPackage(
        NAME CPMLicenses.cmake
        GITHUB_REPOSITORY cpm-cmake/CPMLicenses.cmake
        VERSION 0.0.7
)
if (CPMLicenses.cmake_ADDED)
    cpm_licenses_create_disclaimer_target(
            write-licenses "${CMAKE_CURRENT_SOURCE_DIR}/3rd/LICENSE" "${CPM_PACKAGES}"
    )
endif ()
# make 时自动在 3rd 文件夹下生成 LICENSE 文件
add_custom_target(3rd_licenses
        ALL
        COMMAND
        make
        write-licenses
)

# gdb
find_program(GDB_EXE gdb)
if (NOT GDB_EXE)
    message(FATAL_ERROR "gdb not found.\n"
            "Following https://www.sourceware.org/gdb/ to install.")
endif ()

# qemu
find_program(QEMU_EXE qemu-system-${TARGET_ARCH})
if (NOT QEMU_EXE)
    message(FATAL_ERROR "qemu-system-${TARGET_ARCH} not found.\n"
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
