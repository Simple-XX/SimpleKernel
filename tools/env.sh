
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# setup.sh for Simple-XX/SimpleKernel.

#!/bin/bash

# env.sh path
TOOLS_DIR=$(cd $(dirname ${BASH_SOURCE:-$0});pwd)
# ARCH: i386, x86_64, raspi2, riscv64
# ARCH="i386"
# ARCH="x86_64" # TODO, cannot use
# ARCH="raspi2"
ARCH="riscv64"
# 虚拟机: bochs, qemu
if [ "${ARCH}" == "i386" ]; then
    SIMULATOR="bochs"
elif [ "${ARCH}" == "x86_64" ]; then
    SIMULATOR="bochs"
else
    SIMULATOR="qemu"
fi
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
    if [ "${ARCH}" == "i386" ]; then
        TOOLS="toolchain_linux_x86_64.cmake"
    elif [ "${ARCH}" == "x86_64" ]; then
        TOOLS="toolchain_linux_x86_64.cmake"
    elif [ "${ARCH}" == "raspi2" ]; then
        TOOLS="toolchain_linux_arm.cmake"
    fi
    GRUB_PATH="$(dirname $(which grub-file))"
    bochsrc="./tools/bochsrc_linux.txt"
elif [ "${OS}" == "Darwin" ]; then
    if [ "${ARCH}" == "i386" ]; then
        TOOLS="toolchain_mac_x86_64.cmake"
    elif [ "${ARCH}" == "x86_64" ]; then
        TOOLS="toolchain_mac_x86_64.cmake"
    elif [ "${ARCH}" == "raspi2" ]; then
        TOOLS="toolchain_mac_arm.cmake"
    elif [ "${ARCH}" == "riscv64" ]; then
        TOOLS="toolchain_mac_riscv.cmake"
    fi
    GRUB_PATH="$(pwd)/tools/grub-2.04/build/grub/bin"
    bochsrc="./tools/bochsrc_mac.txt"
fi
