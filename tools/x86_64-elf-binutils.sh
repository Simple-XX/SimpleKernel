
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# i386-elf-binutils.sh for Simple-XX/SimpleKernel.

#!/bin/bash

if ! [ -x "$(command -v wget)" ]; then
    echo 'Error: wget is not installed.'
    exit 1
elif ! [ -x "$(command -v tar)" ]; then
    echo 'Error: tar is not installed.'
    exit 1
else
    wget https://ftp.gnu.org/gnu/binutils/binutils-2.34.tar.xz
    tar zxvf binutils-2.34.tar.xz
    cd binutils-2.34
    ./configure \
        --target=x86_64-elf \
        --enable-targets=all \
        --enable-multilib \
        --enable-64-bit-bfd \
        --disable-werror
    make
    make install
fi
