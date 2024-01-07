
# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# functions.cmake for Simple-XX/SimpleKernel.
# 辅助函数

# 生成 target 输出文件的 readelf -a
# _target: target 名
# 在 ${${_target}_BINARY_DIR} 目录下生成 $<TARGET_FILE:${_target}>.readelf 文件
function(readelf_a _target)
    add_custom_command(TARGET ${_target}
            COMMENT "readelf -a $<TARGET_FILE:${_target}> ..."
            POST_BUILD
            DEPENDS ${_target}
            WORKING_DIRECTORY ${${_target}_BINARY_DIR}
            COMMAND ${CMAKE_READELF} -a $<TARGET_FILE:${_target}> > $<TARGET_FILE:${_target}>.readelf || (exit 0)
    )
endfunction()

# 生成 target 输出文件的 objdump -D
# _target: target 名
# 在 ${${_target}_BINARY_DIR} 目录下生成 $<TARGET_FILE:${_target}>.disassembly 文件
function(objdump_D _target)
    add_custom_command(TARGET ${_target}
            COMMENT "objdump -D $<TARGET_FILE:${_target}> ..."
            POST_BUILD
            DEPENDS ${_target}
            WORKING_DIRECTORY ${${_target}_BINARY_DIR}
            COMMAND ${CMAKE_OBJDUMP} -D $<TARGET_FILE:${_target}> > $<TARGET_FILE:${_target}>.disassembly
    )
endfunction()

# 将 elf 转换为 efi
# _elf: 要转换的 target 名
# _efi: 输出的 efi 文件名
# 在 ${${_target}_BINARY_DIR} 目录下生成 ${_efi} 文件
function(elf2efi _target _efi)
    add_custom_command(TARGET ${_target}
            COMMENT "Convert $<TARGET_FILE:${_target}> to efi ..."
            POST_BUILD
            DEPENDS ${_target}
            WORKING_DIRECTORY ${${_target}_BINARY_DIR}
            COMMAND ${CMAKE_OBJCOPY} $<TARGET_FILE:${_target}> ${_efi}
            -S
            -R .comment
            -R .note.gnu.build-id
            -R .gnu.hash
            -R .dynsym
            --target=efi-app-${TARGET_ARCH} --subsystem=10
    )
endfunction()

# 添加测试覆盖率 target
# DEPENDS 要生成的 targets
# SOURCE_DIR 源码路径
# BINARY_DIR 二进制文件路径
# EXCLUDE_DIR 要排除的目录
function(add_coverage_target)
    # 解析参数
    set(options)
    set(one_value_keywords SOURCE_DIR BINARY_DIR)
    set(multi_value_keywords DEPENDS EXCLUDE_DIR)
    cmake_parse_arguments(
            ARG "${options}" "${one_value_keywords}" "${multi_value_keywords}" ${ARGN}
    )

    # 不检查的目录
    list(APPEND EXCLUDES --exclude)
    foreach (_item ${ARG_EXCLUDE_DIR})
        list(APPEND EXCLUDES '${_item}')
    endforeach ()

    # 添加 target
    add_custom_target(coverage DEPENDS ${ARG_DEPENDS}
            COMMAND ${CMAKE_CTEST_COMMAND}
    )
    # 在 coverage 执行完毕后生成报告
    add_custom_command(TARGET coverage
            COMMENT "Generating coverage report ..."
            POST_BUILD
            WORKING_DIRECTORY ${ARG_BINARY_DIR}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_OUTPUT_DIR}
            COMMAND ${LCOV_EXE}
            -c
            -o ${COVERAGE_OUTPUT_DIR}/coverage.info
            -d ${ARG_BINARY_DIR}
            -b ${ARG_SOURCE_DIR}
            --no-external
            ${EXCLUDES}
            --rc lcov_branch_coverage=1
            COMMAND ${GENHTML_EXE}
            ${COVERAGE_OUTPUT_DIR}/coverage.info
            -o ${COVERAGE_OUTPUT_DIR}
            --branch-coverage
    )
endfunction()

# 添加运行 qemu target
# NAME 生成的 target 前缀
# TARGET 目标架构
# WORKING_DIRECTORY 工作目录
# BOOT boot 文件路径
# KERNEL kernel 文件路径
# DEPENDS 依赖的 target
# QEMU_FLAGS qemu 参数
function(add_run_target)
    # 解析参数
    set(options)
    set(one_value_keywords NAME TARGET WORKING_DIRECTORY BOOT KERNEL)
    set(multi_value_keywords DEPENDS QEMU_FLAGS)
    cmake_parse_arguments(
            ARG "${options}" "${one_value_keywords}" "${multi_value_keywords}" ${ARGN}
    )

    list(APPEND commands
            COMMAND ${CMAKE_COMMAND} -E copy ${ARG_KERNEL} image/
    )
    if (${ARG_TARGET} STREQUAL "x86_64")
        get_filename_component(BOOT_FILE_NAME ${ARG_BOOT} NAME)
        configure_file(${CMAKE_SOURCE_DIR}/tools/startup.nsh.in image/startup.nsh @ONLY)
        list(APPEND commands
                COMMAND ${CMAKE_COMMAND} -E copy ${ARG_BOOT} image/
        )
    elseif (${ARG_TARGET} STREQUAL "aarch64")
        get_filename_component(BOOT_FILE_NAME ${ARG_BOOT} NAME)
        configure_file(${CMAKE_SOURCE_DIR}/tools/startup.nsh.in image/startup.nsh @ONLY)
        list(APPEND commands
                COMMAND ${CMAKE_COMMAND} -E copy ${ARG_BOOT} image/
        )
    endif ()

    # 添加 target
    add_custom_target(${ARG_NAME}_run DEPENDS ${ARG_DEPENDS}
            WORKING_DIRECTORY ${ARG_WORKING_DIRECTORY}
            COMMAND ${CMAKE_COMMAND} -E make_directory image/
            ${commands}
            COMMAND
            qemu-system-${ARG_TARGET}
            ${ARG_QEMU_FLAGS}
    )
    add_custom_target(${ARG_NAME}_debug DEPENDS ${ARG_DEPENDS}
            WORKING_DIRECTORY ${ARG_WORKING_DIRECTORY}
            COMMAND ${CMAKE_COMMAND} -E make_directory image/
            ${commands}
            COMMAND
            qemu-system-${ARG_TARGET}
            ${ARG_QEMU_FLAGS}
            # 等待 gdb 连接
            -S
            # 使用 1234 端口
            -gdb ${QEMU_GDB_PORT}
    )
endfunction()