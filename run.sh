
# This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
#
# setup.sh for MRNIU/SimpleKernel.

#!/usr/bin/env bash
# shell 执行出错时终止运行
set -e
# 输出实际执行内容
# set -x

source ./tools/env.sh

# 重新编译
mkdir -p ./src/build/
cd src/build
cmake -DCMAKE_TOOLCHAIN_FILE=/Users/nzh/Documents/GitHub/SimpleKernel_be23012/src/cmake/toolchain_mac_x86_64.cmake -DPLATFORM=bochs -DARCH=x86_64 ..
make
cd ../../

if ${GRUB_PATH}/grub-file --is-x86-multiboot2 ${kernel}; then
    echo Multiboot2 Confirmed!
elif [ ${ARCH} == "x86_64" ]; then
    if ${GEUB}-file --is-x86-multiboot2 ${bootloader}; then
        echo Multiboot2 Confirmed!
    else
        echo The File is Not Multiboot.
        exit
    fi
fi

# 检测路径是否合法，发生过 rm -rf -f /* 的惨剧
if [ "${iso_boot}" == "" ]; then
    echo iso_boot path error.
else
    rm -rf -f ${iso_boot}/*
fi
cp ${kernel} ${iso_boot}
mkdir ${iso_boot_grub}
touch ${iso_boot_grub}/grub.cfg
if [ ${ARCH} == "x86_64" ]; then
    cp ${bootloader} ${iso_boot}
    echo 'set timeout=15
    set default=0
    menuentry "SimpleKernel" {
       multiboot2 /boot/bootloader.bin
       module /boot/kernel.bin "KERNEL_BIN"
   }' >${iso_boot_grub}/grub.cfg
else
    echo 'set timeout=15
    set default=0
    menuentry "SimpleKernel" {
       multiboot2 /boot/kernel.bin "KERNEL_BIN"
   }' >${iso_boot_grub}/grub.cfg
fi
${GRUB_PATH}/grub-mkrescue -o ${iso} ${iso_folder}
${SIMULATOR} -q -f ${bochsrc} -rc ./tools/bochsinit
