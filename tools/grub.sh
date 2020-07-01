
# This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
#
# i386-elf-grub.sh for MRNIU/SimpleKernel.

#!/usr/bin/env bash

if [ -x "$(command -v apt-get)" ]; then
    sudo apt-get install grub2-common
elif [ -x "$(command -v yum)" ]; then
    sudo yum -S g
elif [ -x "$(command -v pacman)" ]; then
    sudo pacman -S g
elif ! [ -x "$(command -v wget)" ]; then
    echo 'Error: wget is not installed.'
    exit 1
elif ! [ -x "$(command -v tar)" ]; then
    echo 'Error: tar is not installed.'
    exit 1
else
    wget https://ftp.gnu.org/gnu/grub/grub-2.04.tar.xz
    tar zxvf grub-2.04.tar.xz
    cd grub-2.04
    ./configure \
        --target=i386-elf \
        --program-prefix=i386-elf- \
        --disable-werror \
        TARGET_CC=i386-elf-gcc \
        TARGET_OBJCOPY=i386-elf-objcopy \
        TARGET_STRIP=i386-elf-strip \
        TARGET_NM=i386-elf-nm \
        TARGET_RANLIB=i386-elf-ranlib
    make
    make install
fi
