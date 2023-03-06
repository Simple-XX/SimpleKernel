
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).

# bochs.sh for Simple-XX/SimpleKernel.

#!/bin/bash

if ! [ -x "$(command -v pkg-config)" ]; then
    echo 'Error: pkg-config is not installed.'
    exit 1
elif ! [ -x "$(command -v sdl2)" ]; then
    echo 'Error: sdl2 is not installed.'
    exit 1
elif ! [ -x "$(command -v wget)" ]; then
    echo 'Error: wget is not installed.'
    exit 1
elif ! [ -x "$(command -v tar)" ]; then
    echo 'Error: tar is not installed.'
    exit 1
else
    wget https://downloads.sourceforge.net/project/bochs/bochs/2.6.9/bochs-2.6.9.tar.gz
    tar zxvf bochs-2.6.9.tar.gz
    cd bochs-2.6.9
    ./configure \
        --with-nogui \
        --enable-disasm \
        --disable-docbook \
        --enable-x86-64 \
        --enable-pci \
        --enable-all-optimizations \
        --enable-plugins \
        --enable-cdrom \
        --enable-a20-pin \
        --enable-fpu \
        --enable-alignment-check \
        --enable-large-ramfile \
        --enable-debugger-gui \
        --enable-readline \
        --enable-iodebug \
        --enable-show-ips \
        --enable-logging \
        --enable-usb \
        --enable-cpu-level=6 \
        --enable-clgd54xx \
        --enable-avx \
        --enable-vmx=2 \
        --enable-long-phy-address \
        --with-term \
        --with-sdl2
    make
    make install
fi
