
# This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
#
# setup.sh for MRNIU/SimpleKernel.

#!/usr/bin/env bash
# shell 执行出错时终止运行
set -e
# 输出实际执行内容
# set -x

TARGET="x86_64-elf"
# ARCH: i386, x86_64
ARCH="i386"
# 虚拟机
SIMULATOR="bochs"
# bochs 配置文件
bochsrc="bochsrc.txt"
# 内核映像
kernel='./src/kernel.bin'
bootloader='./src/bootloader.bin'
# 软盘
disk='simplekernel.img'
# 安装目录
boot_folder='./boot_folder/boot/'
# 挂载目录
folder='./boot_folder'
# 工具目录
tool='./tools'
iso_boot_grub='./iso/boot/grub'
iso_boot='./iso/boot/'
iso='./simplekernel.iso'
iso_folder='./iso/'
# 判断操作系统类型
OS=`uname -s`

# 检测环境，如果没有安装需要的软件，则安装
if ! [ -x "$(command -v ${SIMULATOR})" ]; then
    echo 'Error: '${SIMULATOR}' is not installed.'
    echo 'Install '${SIMULATOR}'...'
    if [ ${OS} == "Darwin" ]; then
        brew install ${SIMULATOR}
    elif [ ${OS} == "Linux" ]; then
        shell ${tool}/${SIMULATOR}.sh
    fi
fi

if ! [ -x "$(command -v ${TARGET}-ld)" ]; then
    echo 'Error: '${TARGET}'-binutils is not installed.'
    echo 'Install '${TARGET}'-binutils...'
    if [ ${OS} == "Darwin" ]; then
        brew install ${TARGET}-binutils
    elif [ ${OS} == "Linux" ]; then
        shell ${tool}/${TARGET}-binutils.sh
    fi
fi

if ! [ -x "$(command -v ${TARGET}-gcc)" ]; then
    echo 'Error: '${TARGET}'-gcc is not installed.'
    echo 'Install '${TARGET}'-gcc...'
    if [ ${OS} == "Darwin" ]; then
        brew install ${TARGET}-gcc
    elif [ ${OS} == "Linux" ]; then
        shell ${tool}/${TARGET}-gcc.sh
    fi
fi

if ! [ -x "$(command -v i386-elf-grub-file)" ]; then
    echo 'Error: i386-elf-grub is not installed.'
    echo 'Install i386-elf-grub...'
    if [ ${OS} == "Darwin" ]; then
        brew install i386-elf-grub
    elif [ ${OS} == "Linux" ]; then
        shell ${tool}/i386-elf-grub.sh
    fi
fi

if ! [ -x "$(command -v xorriso)" ]; then
    echo 'Error: xorriso is not installed.'
    echo 'Install xorriso...'
    if [ ${OS} == "Darwin" ]; then
        brew install xorriso
    elif [ ${OS} == "Linux" ]; then
        shell ${tool}/xorriso.sh
    fi
fi

# 重新编译
cd src/
make remake
cd ../

if i386-elf-grub-file --is-x86-multiboot2 ${kernel}; then
    echo Multiboot2 Confirmed!
elif [ ${ARCH} == "x86_64" ]; then
    if i386-elf-grub-file --is-x86-multiboot2 ${bootloader}; then
        echo Multiboot2 Confirmed!
    else
        echo The File is Not Multiboot.
        exit
    fi
fi

rm -rf -f ${iso_boot}/*
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
${TARGET}-grub-mkrescue -o ${iso} ${iso_folder}
${SIMULATOR} -q -f ${bochsrc} -rc ./tools/bochsinit
