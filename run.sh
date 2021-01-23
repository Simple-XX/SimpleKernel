
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# setup.sh for Simple-XX/SimpleKernel.

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
cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/${TOOLS} -DPLATFORM=${SIMULATOR} -DARCH=${ARCH} -DCMAKE_BUILD_TYPE=DEBUG ..
make
cd ../


if [ ${ARCH} == "i386" ]; then
    if ${GRUB_PATH}/grub-file --is-x86-multiboot2 ${kernel}; then
        echo Multiboot2 Confirmed!
    fi
elif [ ${ARCH} == "x86_64" ]; then
    if ${GRUB_PATH}/grub-file --is-x86-multiboot2 ${kernel}_boot; then
        echo Multiboot2 Confirmed!
    fi
elif [ ${ARCH} == "raspi2" ]; then
    echo Arm-A7.
else
    echo The File is Not Multiboot.
    exit
fi

# 检测路径是否合法，发生过 rm -rf -f /* 的惨剧
if [ "${iso_boot}" == "" ]; then
    echo iso_boot path error.
else
    mkdir -p ${iso_boot}
    rm -rf -f ${iso_boot}/*
fi

cp ${kernel} ${iso_boot}
if [ ${ARCH} == "x86_64" ]; then
    cp ${kernel}_boot ${iso_boot}
fi
mkdir ${iso_boot_grub}
touch ${iso_boot_grub}/grub.cfg

if [ ${ARCH} == "i386" ]; then
    echo 'set timeout=15
    set default=0
    menuentry "SimpleKernel" {
       multiboot2 /boot/kernel.elf "KERNEL_ELF"
   }' >${iso_boot_grub}/grub.cfg
fi

if [ ${ARCH} == "x86_64" ]; then
    echo 'set timeout=15
    set default=0
    menuentry "SimpleKernel" {
       multiboot2 /boot/kernel.elf_boot "KERNEL_ELF_boot"
   }' >${iso_boot_grub}/grub.cfg
fi

if [ ${ARCH} == "i386" ]; then
    ${GRUB_PATH}/grub-mkrescue -o ${iso} ${iso_folder}
    ${SIMULATOR} -q -f ${bochsrc} -rc ./tools/bochsinit
elif [ ${ARCH} == "x86_64" ]; then
    ${GRUB_PATH}/grub-mkrescue -o ${iso} ${iso_folder}
    ${SIMULATOR} -q -f ${bochsrc} -rc ./tools/bochsinit
elif [ ${ARCH} == "raspi2" ]; then
    ${SIMULATOR}-system-arm -machine raspi2 -serial stdio -kernel ${kernel} 
fi
