
# This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
#
# setup.sh for MRNIU/SimpleKernel.

#!/usr/bin/env bash

# env.sh path
TOOLS_DIR=$(cd $(dirname ${BASH_SOURCE:-$0});pwd)

TARGET="x86_64-elf"
# ARCH: i386, x86_64
ARCH="i386"
# 虚拟机
SIMULATOR="bochs"
# grub
GEUB="i386-elf-grub"
# bochs 配置文件
bochsrc="bochsrc.txt"
# 内核映像
kernel=${TOOLS_DIR}'/../src/build/bin/kernel.bin'
echo ${kernel}
bootloader=${TOOLS_DIR}'/../src/build/bin/bootloader.bin'
# 软盘
disk=${TOOLS_DIR}'/../simplekernel.img'
# 安装目录
boot_folder=${TOOLS_DIR}'/../boot_folder/boot/'
# 挂载目录
folder=${TOOLS_DIR}'/../boot_folder'
iso_boot_grub=${TOOLS_DIR}'/../iso/boot/grub'
iso_boot=${TOOLS_DIR}'/../iso/boot/'
iso=${TOOLS_DIR}'/../simplekernel.iso'
iso_folder=${TOOLS_DIR}'/../iso/'
# 判断操作系统类型
OS=`uname -s`

IS_SIMULATOR=$(command -v ${SIMULATOR})
