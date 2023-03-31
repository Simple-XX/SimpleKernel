#!/bin/bash

# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# run.sh for Simple-XX/SimpleKernel.
# 在虚拟机中运行内核

# shell 执行出错时终止运行
set -e
# 输出实际执行内容
#set -x

source ./tools/env.sh
export PATH="${GRUB_PATH}:$PATH"

# 重新编译
mkdir -p ./build/
rm -rf ./build/*
cd ./build
cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/${TOOLS} -DARCH=${ARCH} -DCMAKE_BUILD_TYPE=DEBUG ..
make
cd ../

# 如果是 i386/x86_64，需要判断是否符合 multiboot2 标准
if [ ${ARCH} == "i386" ] || [ ${ARCH} == "x86_64" ]; then
    if ${GRUB_PATH}/grub-file --is-x86-multiboot2 ${kernel}; then
        echo Multiboot2 Confirmed!
    else
        echo NOT Multiboot2!
        exit
    fi
fi

# 如果是 riscv 64，需要使用 opensbi
if [ ${ARCH} == "riscv64" ]; then
    # OPENSBI 不存在则编译
    if [ ! -f ${OPENSBI} ]; then
        echo build opensbi.
        cd ./tools/opensbi
        mkdir -p build
        export CROSS_COMPILE=${TOOLCHAIN_PREFIX}
        export FW_JUMP=y
        export FW_JUMP_ADDR=0x80200000
        export PLATFORM_RISCV_XLEN=64
        export PLATFORM=generic
        make
        cd ../..
        echo build opensbi done.
    fi
fi

# 检测路径是否合法，发生过 rm -rf -f /* 的惨剧
if [ "${iso_boot}" == "" ]; then
    echo iso_boot path error.
else
    mkdir -p ${iso_boot}
    rm -rf -f ${iso_boot}/*
fi

# 初始化 gdb
if [ ${DEBUG} == 1 ]; then
    if [ ${ARCH} == "i386" ]; then
        cp ./tools/gdbinit_i386 ./.gdbinit
    elif [ ${ARCH} == "x86_64" ]; then
        cp ./tools/gdbinit_x86_64 ./.gdbinit
    elif [ ${ARCH} == "aarch64" ]; then
        cp ./tools/gdbinit_aarch64 ./.gdbinit
    elif [ ${ARCH} == "riscv64" ]; then
        cp ./tools/gdbinit_riscv64 ./.gdbinit
    fi
    echo "target remote localhost:1234" >> ./.gdbinit
    GDB_OPT='-S -gdb tcp::1234'
    echo "Run GDB in another shell"
fi

# 设置 grub 相关数据
if [ ${ARCH} == "i386" ] || [ ${ARCH} == "x86_64" ]; then
    mkdir -p ${iso_boot_grub}
    cp ${kernel} ${iso_boot}
    cp ./tools/grub.cfg ${iso_boot_grub}/
fi

# 运行虚拟机
if [ ${ARCH} == "i386" ] || [ ${ARCH} == "x86_64" ]; then
    if [ ${IA32_USE_QEMU} == 0 ]; then
        ${GRUB_PATH}/grub-mkrescue -o ${iso} ${iso_folder}
        bochs -q -f ${bochsrc} -rc ./tools/bochsinit
    else
        qemu-system-x86_64 -cdrom ${iso} -m 128M \
        -monitor telnet::2333,server,nowait -serial stdio \
        ${GDB_OPT}
    fi
elif [ ${ARCH} == "aarch64" ]; then
    qemu-system-aarch64 -machine virt -cpu cortex-a72 -kernel ${kernel} \
    -monitor telnet::2333,server,nowait -serial stdio -nographic \
    ${GDB_OPT}
elif [ ${ARCH} == "riscv64" ]; then
    qemu-system-riscv64 -machine virt -bios ${OPENSBI} -kernel ${kernel} \
    -monitor telnet::2333,server,nowait -serial stdio -nographic \
    ${GDB_OPT}
fi
