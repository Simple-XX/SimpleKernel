
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# setup.sh for Simple-XX/SimpleKernel.

#!/bin/bash

# env.sh path
TOOLS_DIR=$(cd $(dirname ${BASH_SOURCE:-$0});pwd)
# ARCH: x86_64, raspi2
ARCH="x86_64"
# ARCH="raspi2"
# 虚拟机: bochs, qemu
SIMULATOR="bochs"
# SIMULATOR="qemu"
# 内核映像
kernel='./build/bin/kernel.elf'
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
    if [ "${ARCH}" == "x86_64" ]; then
        TOOLS="toolchain_linux_x86_64.cmake"
    elif [ "${ARCH}" == "raspi2" ]; then
        TOOLS="toolchain_linux_arm.cmake"
    fi
    GRUB_PATH="$(dirname $(which grub-file))"
    bochsrc="./tools/bochsrc_linux.txt"
elif [ "${OS}" == "Darwin" ]; then
    if [ "${ARCH}" == "x86_64" ]; then
        TOOLS="toolchain_mac_x86_64.cmake"
    elif [ "${ARCH}" == "raspi2" ]; then
        TOOLS="toolchain_mac_arm.cmake"
    fi
    GRUB_PATH="$(pwd)/tools/grub-2.04/build/grub/bin"
    bochsrc="./tools/bochsrc_mac.txt"
fi