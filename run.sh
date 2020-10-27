
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
cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/${TOOLS} -DPLATFORM=${SIMULATOR} -DARCH=${ARCH} ..
make
cd ../

if ${GRUB_PATH}/grub-file --is-x86-multiboot2 ${kernel}; then
    echo Multiboot2 Confirmed!
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
mkdir ${iso_boot_grub}
touch ${iso_boot_grub}/grub.cfg

if [ ${ARCH} == "x86_64" ]; then
    echo 'set timeout=15
    set default=0
    menuentry "SimpleKernel" {
       multiboot2 /boot/kernel.bin "KERNEL_BIN"
   }' >${iso_boot_grub}/grub.cfg
fi

${GRUB_PATH}/grub-mkrescue -o ${iso} ${iso_folder}

if [ ${SIMULATOR} == "bochs" ]; then
    bochs -q -f ${bochsrc} -rc ./tools/bochsinit
elif [ ${SIMULATOR} == "qemu" ]; then
        qemu-system-${ARCH} -boot d -cdrom simplekernel.iso -m 512
else
    echo The SIMULATOR is invalid.
    exit
fi
