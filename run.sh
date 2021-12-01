
# This file is a part of Simple-XX/SimpleKernel 
# (https://github.com/Simple-XX/SimpleKernel).
#
# run.sh for Simple-XX/SimpleKernel.
# 在虚拟机中运行内核

#!/bin/bash
# shell 执行出错时终止运行
set -e
# 输出实际执行内容
# set -x

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
        git submodule init
        git submodule update
        cd ./tools/opensbi
        mkdir -p build
        export CROSS_COMPILE=${TOOLCHAIN_PREFIX}
        make PLATFORM=generic FW_JUMP_ADDR=0x80200000
        cd ../..
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
if [ ! -f ./fatfs.dmg ]; then
    # osx 下需要手动制作
    echo NO fatfs.dmg!
    exit
fi

# 设置 grub 相关数据
if [ ${ARCH} == "i386" ] || [ ${ARCH} == "x86_64" ]; then
    cp ${kernel} ${iso_boot}
    mkdir ${iso_boot_grub}
    touch ${iso_boot_grub}/grub.cfg
    echo 'set timeout=15
    set default=0
    menuentry "SimpleKernel" {
       multiboot2 /boot/kernel.elf "KERNEL_ELF"
   }' >${iso_boot_grub}/grub.cfg
fi

# 运行虚拟机
if [ ${ARCH} == "i386" ] || [ ${ARCH} == "x86_64" ]; then
    if [ ${IA32_USE_QEMU} == 0 ]; then
        ${GRUB_PATH}/grub-mkrescue -o ${iso} ${iso_folder}
        bochs -q -f ${bochsrc} -rc ./tools/bochsinit
    else
        qemu-system-x86_64 -cdrom ${iso} -m 128M \
        -monitor telnet::2333,server,nowait -serial stdio
    fi
elif [ ${ARCH} == "aarch64" ]; then
    qemu-system-aarch64 -machine virt -cpu cortex-a72 -kernel ${kernel} \
    -monitor telnet::2333,server,nowait -serial stdio -nographic
elif [ ${ARCH} == "riscv64" ]; then
    qemu-system-riscv64 -machine virt -bios ${OPENSBI} -kernel ${kernel} \
    -global virtio-mmio.force-legacy=false \
    -device virtio-blk-device,bus=virtio-mmio-bus.0,drive=fatfs \
    -drive file=mydisk,format=raw,id=fatfs \
    -device virtio-scsi-device,bus=virtio-mmio-bus.1,id=scsi \
    -drive file=fatfs.dmg,format=raw,id=scsi \
    -monitor telnet::2333,server,nowait -serial stdio -nographic
fi

# qemu-system-riscv64 -machine virt -bios ${OPENSBI} -kernel ${kernel} \
# -global virtio-mmio.force-legacy=false \
# -drive file=mydisk,if=none,format=raw,id=hd \
# -device virtio-blk-device,drive=hd \
# -monitor telnet::2333,server,nowait -serial stdio -nographic
