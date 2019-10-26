
# This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

# setup.sh for MRNIU/SimpleKernel.

#!/usr/bin/env bash
# shell 执行出错时终止运行
set -e
# 输出实际执行内容
# set -x

TARGET="x86_64-elf"
ARCH="x86_64"

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

iso_boot='./iso/boot/'
iso='./simplekernel.iso'
iso_folder='./iso/'
# 判断操作系统类型
OS=0
which_os=`uname -s`
if [ ${which_os} == "Darwin" ]; then
    OS=0
elif [ ${which_os} == "Linux" ]; then
    OS=1
fi

# 检测环境，如果没有安装需要的软件，则安装
if ! [ -x "$(command -v bochs)" ]; then
    echo 'Error: bochs is not installed.'
    echo 'Install bochs...'
    if [ ${OS} == 0 ]; then
        brew install bochs
    elif [ ${OS} == 1 ]; then
        shell ${tool}/bochs.sh
    fi
fi

if ! [ -x "$(command -v ${TARGET}-ld)" ]; then
    echo 'Error: '${TARGET}'-binutils is not installed.'
    echo 'Install '${TARGET}'-binutils...'
    if [ ${OS} == 0 ]; then
        brew install ${TARGET}-binutils
    elif [ ${OS} == 1 ]; then
        shell ${tool}/i386-elf-binutils.sh
    fi
fi

if ! [ -x "$(command -v ${TARGET}-gcc)" ]; then
    echo 'Error: '${TARGET}'-gcc is not installed.'
    echo 'Install '${TARGET}'-gcc...'
    if [ ${OS} == 0 ]; then
        brew install ${TARGET}-gcc
    elif [ ${OS} == 1 ]; then
        shell ${tool}/${TARGET}-gcc.sh
    fi
fi

if ! [ -x "$(command -v x86_64-elf-grub-file)" ]; then
    echo 'Error: '${TARGET}'-grub is not installed.'
    echo 'Install '${TARGET}'-grub...'
    if [ ${OS} == 0 ]; then
        brew install ${TARGET}-grub
    elif [ ${OS} == 1 ]; then
        shell ${tool}/${TARGET}-grub.sh
    fi
fi

if ! [ -x "$(command -v xorriso)" ]; then
    echo 'Error: xorriso is not installed.'
    echo 'Install xorriso...'
    if [ ${OS} == 0 ]; then
        brew install xorriso
    elif [ ${OS} == 1 ]; then
        shell ${tool}/xorriso.sh
    fi
fi

# 重新编译
cd src/
make remake
cd ../

if x86_64-elf-grub-file --is-x86-multiboot2 ${bootloader}; then
    echo Multiboot2 Confirmed!
else
    echo the file is not multiboot
    exit
fi

cp ${kernel} ${iso_boot}
cp ${bootloader} ${iso_boot}
x86_64-elf-grub-mkrescue -o ${iso} ${iso_folder}
bochs -q -f ${bochsrc}
