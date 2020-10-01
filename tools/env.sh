
# This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
#
# setup.sh for MRNIU/SimpleKernel.

#!/usr/bin/env bash

# env.sh path
TOOLS_DIR=$(cd $(dirname ${BASH_SOURCE:-$0});pwd)
TARGET="x86_64-elf"
# ARCH: i386, x86_64
ARCH="x86_64"
# 虚拟机
SIMULATOR="bochs"
# bochs 配置文件
bochsrc="./tools/bochsrc.txt"
# 内核映像
kernel='./build/bin/kernel.bin'
bootloader='./build/bin/bootloader.bin'
# 软盘
disk='./simplekernel.img'
# 安装目录
boot_folder='./boot_folder/boot/'
# 挂载目录
folder='./boot_folder'
iso_boot_grub='./iso/boot/grub'
iso_boot='./iso/boot/'
iso='./simplekernel.iso'
iso_folder='./iso/'
# 判断操作系统类型
OS=`uname -s`
# toolchain
if [ "${OS}" == "Linux" ]; then
    TOOLS="toolchain_linux_x86_64.cmake"
    GRUB_PATH="$(dirname $(which grub-file))"
else if [ "${OS}" == "Darwin" ]; then
    TOOLS="toolchain_mac_x86_64.cmake"
    GRUB_PATH="$(pwd)/tools/grub-2.04/build/grub/bin"
    fi
fi
