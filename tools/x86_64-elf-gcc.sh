
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# x86_64-elf-gcc.sh for Simple-XX/SimpleKernel.

#!/bin/bash

if ! [ -x "$(command -v x86_64-elf-ld)" ]; then
    echo 'Error: i386-elf-binutils is not installed.'
    exit 1
elif ! [ -x "$(command -v wget)" ]; then
    echo 'Error: wget is not installed.'
    exit 1
elif ! [ -x "$(command -v tar)" ]; then
    echo 'Error: tar is not installed.'
    exit 1
else
    wget https://ftp.gnu.org/gnu/gcc/gcc-10.1.0/gcc-10.1.0.tar.xz
    tar zxvf gcc-10.1.0.tar.xz
    cd gcc-10.1.0
    ./configure \
        --target=x86_64-elf \
        --enable-targets=all \
        --enable-multilib \
        --without-isl \
        --disable-werror \
        --without-headers \
        --with-as=x86_64-elf-as \
        --with-ld=x86_64-elf-ld \
        --enable-languages=c,c++
    make all-gcc
    make install-gcc
    make all-target-libgcc
    make install-target-libgcc
fi
