# Copyright The SimpleKernel Contributors

# 添加测试覆盖率 target
# DEPENDS 要生成的 targets
# SOURCE_DIR 源码路径
# BINARY_DIR 二进制文件路径
# EXCLUDE_DIR 要排除的目录
FUNCTION(add_coverage_target)
    # 解析参数
    SET (options)
    SET (one_value_keywords SOURCE_DIR BINARY_DIR)
    SET (multi_value_keywords DEPENDS EXCLUDE_DIR)
    CMAKE_PARSE_ARGUMENTS (ARG "${options}" "${one_value_keywords}"
                           "${multi_value_keywords}" ${ARGN})

    # 不检查的目录
    LIST (APPEND EXCLUDES)
    FOREACH(item ${ARG_EXCLUDE_DIR})
        LIST (APPEND EXCLUDES '${item}')
    ENDFOREACH()

    # 添加 target
    ADD_CUSTOM_TARGET (
        coverage
        COMMENT ""
        DEPENDS ${ARG_DEPENDS}
        COMMAND ${CMAKE_CTEST_COMMAND})

    # 在 coverage 执行完毕后生成报告
    ADD_CUSTOM_COMMAND (
        TARGET coverage
        COMMENT "Generating coverage report ..."
        POST_BUILD
        WORKING_DIRECTORY ${ARG_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_OUTPUT_DIR}
        COMMAND
            ${LCOV_EXE} -c -o ${COVERAGE_OUTPUT_DIR}/coverage.info -d
            ${ARG_BINARY_DIR} -b ${ARG_SOURCE_DIR} ${EXCLUDES} --rc
            branch_coverage=1 --ignore-errors mismatch
        COMMAND ${GENHTML_EXE} ${COVERAGE_OUTPUT_DIR}/coverage.info -o
                ${COVERAGE_OUTPUT_DIR} --branch-coverage)
ENDFUNCTION()

# 添加在 qemu 中运行内核
# DEPENDS 依赖的 target
# QEMU_FLAGS qemu 参数
FUNCTION(add_run_target)
    # 解析参数
    SET (options)
    SET (one_value_keywords NAME TARGET)
    SET (multi_value_keywords DEPENDS QEMU_BOOT_FLAGS)
    CMAKE_PARSE_ARGUMENTS (ARG "${options}" "${one_value_keywords}"
                           "${multi_value_keywords}" ${ARGN})

    # 获取目标文件信息
    ADD_CUSTOM_COMMAND (
        TARGET ${ARG_TARGET}
        POST_BUILD
        COMMENT "Generating binary info for $<TARGET_FILE_NAME:${ARG_TARGET}>"
        VERBATIM
        WORKING_DIRECTORY $<TARGET_FILE_DIR:${ARG_TARGET}>
        COMMAND
            ${CMAKE_OBJDUMP} -D $<TARGET_FILE:${ARG_TARGET}> >
            $<TARGET_FILE_DIR:${ARG_TARGET}>/$<TARGET_FILE_NAME:${ARG_TARGET}>.objdump
            || true
        COMMAND
            ${CMAKE_READELF} -a $<TARGET_FILE:${ARG_TARGET}> >
            $<TARGET_FILE_DIR:${ARG_TARGET}>/$<TARGET_FILE_NAME:${ARG_TARGET}>.readelf
            || true
        COMMAND
            ${CMAKE_NM} -a $<TARGET_FILE:${ARG_TARGET}> >
            $<TARGET_FILE_DIR:${ARG_TARGET}>/$<TARGET_FILE_NAME:${ARG_TARGET}>.nm
        COMMAND
            ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${ARG_TARGET}>
            $<TARGET_FILE_DIR:${ARG_TARGET}>/$<TARGET_FILE_NAME:${ARG_TARGET}>.bin
    )

    # 生成 rootfs.img
    ADD_CUSTOM_COMMAND (
        OUTPUT ${CMAKE_BINARY_DIR}/bin/rootfs.img
        COMMENT "Generating rootfs.img ..."
        VERBATIM
        WORKING_DIRECTORY $<TARGET_FILE_DIR:${ARG_TARGET}>
        COMMAND dd if=/dev/zero of=${CMAKE_BINARY_DIR}/bin/rootfs.img bs=1M
                count=64
        COMMAND mkfs.fat -F 32 ${CMAKE_BINARY_DIR}/bin/rootfs.img)

    # 生成 QEMU DTS 和 DTB
    ADD_CUSTOM_COMMAND (
        OUTPUT ${CMAKE_BINARY_DIR}/bin/qemu.dtb ${CMAKE_BINARY_DIR}/bin/qemu.dts
        COMMENT "Generating QEMU DTS and DTB ..."
        VERBATIM
        DEPENDS ${CMAKE_BINARY_DIR}/bin/rootfs.img
        WORKING_DIRECTORY $<TARGET_FILE_DIR:${ARG_TARGET}>
        COMMAND
            qemu-system-${CMAKE_SYSTEM_PROCESSOR} ${QEMU_COMMON_FLAG}
            ${QEMU_DEVICE_FLAGS} ${QEMU_MACHINE_FLAGS} -machine
            dumpdtb=$<TARGET_FILE_DIR:${ARG_TARGET}>/qemu.dtb
        COMMAND dtc -I dtb $<TARGET_FILE_DIR:${ARG_TARGET}>/qemu.dtb -O dts -o
                $<TARGET_FILE_DIR:${ARG_TARGET}>/qemu.dts)

    # 生成 U-BOOT FIT
    ADD_CUSTOM_TARGET (
        ${ARG_TARGET}_gen_fit
        COMMENT "Generating U-BOOT FIT ..."
        VERBATIM
        WORKING_DIRECTORY $<TARGET_FILE_DIR:${ARG_TARGET}>
        DEPENDS
            $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},aarch64>:$<TARGET_FILE_DIR:${ARG_TARGET}>/qemu.dtb>
            $<$<STREQUAL:${CMAKE_SYSTEM_PROCESSOR},riscv64>:$<TARGET_FILE_DIR:${ARG_TARGET}>/qemu.dtb>
            ${ARG_TARGET}
        COMMAND
            ${CMAKE_COMMAND} -D
            IN_FILE=${CMAKE_SOURCE_DIR}/tools/${CMAKE_SYSTEM_PROCESSOR}_qemu_virt.its.in
            -D OUT_FILE=${CMAKE_BINARY_DIR}/bin/boot.its -D
            KV_PAIRS=DESC\;$<TARGET_FILE_NAME:${ARG_TARGET}>\;KERNEL_PATH\;$<TARGET_FILE:${ARG_TARGET}>\;DTB_PATH\;$<TARGET_FILE_DIR:${ARG_TARGET}>/qemu.dtb;
            -P ${CMAKE_SOURCE_DIR}/cmake/replace_kv.cmake
        COMMAND mkimage -f $<TARGET_FILE_DIR:${ARG_TARGET}>/boot.its
                $<TARGET_FILE_DIR:${ARG_TARGET}>/boot.fit || true
        COMMAND
            mkimage -T script -d
            ${CMAKE_SOURCE_DIR}/tools/${CMAKE_SYSTEM_PROCESSOR}_boot_scr.txt
            $<TARGET_FILE_DIR:${ARG_TARGET}>/boot.scr.uimg
        COMMAND ${CMAKE_COMMAND} -E make_directory /srv/tftp
        COMMAND ln -s -f $<TARGET_FILE_DIR:${ARG_TARGET}>/boot.scr.uimg
                /srv/tftp/boot.scr.uimg
        COMMAND ln -s -f $<TARGET_FILE_DIR:${ARG_TARGET}> /srv/tftp)

    # 添加 target
    ADD_CUSTOM_TARGET (
        ${ARG_NAME}run
        COMMENT "Run $<TARGET_FILE_NAME:${ARG_TARGET}> ..."
        DEPENDS ${ARG_DEPENDS} ${ARG_TARGET}_gen_fit
                ${CMAKE_BINARY_DIR}/bin/rootfs.img
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMAND
            qemu-system-${CMAKE_SYSTEM_PROCESSOR} ${QEMU_COMMON_FLAG}
            ${QEMU_DEVICE_FLAGS} ${QEMU_MACHINE_FLAGS} ${ARG_QEMU_BOOT_FLAGS})
    ADD_CUSTOM_TARGET (
        ${ARG_NAME}debug
        COMMENT "Debug $<TARGET_FILE_NAME:${ARG_TARGET}> ..."
        DEPENDS ${ARG_DEPENDS} ${ARG_TARGET}_gen_fit
                ${CMAKE_BINARY_DIR}/bin/rootfs.img
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMAND
            qemu-system-${CMAKE_SYSTEM_PROCESSOR} ${QEMU_COMMON_FLAG}
            ${QEMU_DEVICE_FLAGS} ${QEMU_MACHINE_FLAGS} ${QEMU_DEBUG_FLAGS}
            ${ARG_QEMU_BOOT_FLAGS})
ENDFUNCTION()
