
# This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

# setup.sh for MRNIU/SimpleKernel.

#!/usr/bin/env bash
# shell 执行出错时终止运行
set -e
# 输出实际执行内容
# set -x

# bochs 配置文件
bochsrc="bochsrc.txt"
# 内核映像
img='./src/kernel.kernel'
# 软盘
disk='simplekernel.img'
# 安装目录
boot_folder='./boot_folder/boot/'
# 挂载目录
folder='./boot_folder'
# 工具目录
tool='./tools'

iso_boot='./iso/boot/kernel.kernel'
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

if ! [ -x "$(command -v i386-elf-ld)" ]; then
    echo 'Error: i386-elf-binutils is not installed.'
    echo 'Install i386-elf-binutils...'
    if [ ${OS} == 0 ]; then
        brew install i386-elf-binutils
    elif [ ${OS} == 1 ]; then
        shell ${tool}/i386-elf-binutils.sh
    fi
fi

if ! [ -x "$(command -v i386-elf-gcc)" ]; then
    echo 'Error: i386-elf-gcc is not installed.'
    echo 'Install i386-elf-gcc...'
    if [ ${OS} == 0 ]; then
        brew install i386-elf-gcc
    elif [ ${OS} == 1 ]; then
        shell ${tool}/i386-elf-gcc.sh
    fi
fi

if ! [ -x "$(command -v i386-elf-grub-file)" ]; then
    echo 'Error: i386-elf-grub is not installed.'
    echo 'Install i386-elf-grub...'
    if [ ${OS} == 0 ]; then
        brew install i386-elf-grub
    elif [ ${OS} == 1 ]; then
        shell ${tool}/i386-elf-grub.sh
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

if i386-elf-grub-file --is-x86-multiboot2 ${img}; then
    echo Multiboot2 Confirmed!
else
    echo the file is not multiboot
    exit
fi

cp ${img} ${iso_boot}
i386-elf-grub-mkrescue -o ${iso} ${iso_folder}
bochs -q -f ${bochsrc}
