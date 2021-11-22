
# This file is a part of Simple-XX/SimpleKernel 
# (https://github.com/Simple-XX/SimpleKernel).
#
# setup.sh for Simple-XX/SimpleKernel.
# 指定要运行的 ARCH，并设置相关数据

#!/bin/bash

# ARCH: i386, x86_64, riscv64
# ARCH="i386"
# ARCH="x86_64"
ARCH="riscv64"

# 内核映像
kernel='./build/bin/kernel.elf'
iso_boot_grub='./iso/boot/grub'
iso_boot='./iso/boot/'
iso='./simplekernel.iso'
iso_folder='./iso/'
# 判断操作系统类型
OS=`uname -s`
# toolchain
if [ "${OS}" == "Linux" ]; then
    if [ "${ARCH}" == "i386" ] || [ "${ARCH}" == "x86_64" ]; then
        TOOLS="toolchain_linux_x86_64.cmake"
    elif [ "${ARCH}" == "arm" ]; then
        TOOLS="toolchain_linux_arm.cmake"
    fi
    GRUB_PATH="$(dirname $(which grub-file))"
    bochsrc="./tools/bochsrc_linux.txt"
elif [ "${OS}" == "Darwin" ]; then
    if [ "${ARCH}" == "i386" ] || [ "${ARCH}" == "x86_64" ]; then
        TOOLS="toolchain_mac_x86_64.cmake"
    elif [ "${ARCH}" == "arm" ]; then
        TOOLS="toolchain_mac_arm.cmake"
    elif [ "${ARCH}" == "riscv64" ]; then
        TOOLS="toolchain_mac_riscv.cmake"
    fi
    OPENSBI="$(pwd)/tools/opensbi/build/platform/generic/firmware/fw_jump.elf"
    GRUB_PATH="$(pwd)/tools/grub-2.04/build/grub/bin"
    bochsrc="./tools/bochsrc_mac.txt"
fi
