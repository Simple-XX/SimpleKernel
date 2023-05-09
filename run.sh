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
mkdir -p ./build_${ARCH}/
rm -rf ./build_${ARCH}/*
cd ./build_${ARCH}
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

# 制作 fatfs 镜像
if [ ! -f ./fatfs.img ]; then
    # osx 下需要手动制作
    echo NO fatfs.img!
    bash ./tools/mkfatfs.sh
    exit
fi

# 初始化 gdb
if [ ${DEBUG} == 1 ]; then
    cp ./tools/gdbinit ./.gdbinit
    echo "" >> ./.gdbinit
    echo "file "${kernel} >> ./.gdbinit
    if [ ${ARCH} == "riscv64" ]; then
        echo "add-symbol-file "${OPENSBI} >> ./.gdbinit
    fi
    echo "target remote localhost:1234" >> ./.gdbinit
    GDB_OPT='-S -gdb tcp::1234'
    echo "Run gdb-multiarch in another shell"
fi

# 设置 grub 相关数据
if [ ${ARCH} == "i386" ] || [ ${ARCH} == "x86_64" ]; then
    mkdir -p ${iso_boot_grub}
    cp ${kernel} ${iso_boot}
    cp ./tools/grub.cfg ${iso_boot_grub}/
fi

# 运行虚拟机
if [ ${ARCH} == "i386" ] || [ ${ARCH} == "x86_64" ]; then
    qemu-system-x86_64 -cdrom ${iso} -m 128M \
    -monitor telnet::2333,server,nowait -serial stdio \
    ${GDB_OPT}
elif [ ${ARCH} == "aarch64" ]; then
    qemu-system-aarch64 -machine virt -cpu cortex-a72 -kernel ${kernel} \
    -monitor telnet::2333,server,nowait -serial stdio -nographic \
    ${GDB_OPT}
elif [ ${ARCH} == "riscv64" ]; then
    qemu-system-riscv64 -machine virt -bios ${OPENSBI} -kernel ${kernel} \
    -global virtio-mmio.force-legacy=false \
    -device virtio-blk-device,bus=virtio-mmio-bus.0,drive=fatfs \
    -drive file=fatfs.img,format=raw,id=fatfs \
    -device virtio-scsi-device,bus=virtio-mmio-bus.1,id=scsi \
    -drive file=fatfs.img,format=raw,id=scsi \
    -monitor telnet::2333,server,nowait -serial stdio -nographic \
    ${GDB_OPT}
fi

# qemu-system-riscv64 -machine virt -bios ${OPENSBI} -kernel ${kernel} \
# -global virtio-mmio.force-legacy=false \
# -drive file=mydisk,if=none,format=raw,id=hd \
# -device virtio-blk-device,drive=hd \
# -monitor telnet::2333,server,nowait -serial stdio -nographic
