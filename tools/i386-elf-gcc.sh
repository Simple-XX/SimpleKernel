
# This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
#
# i386-elf-gcc.sh for MRNIU/SimpleKernel.

#!/usr/bin/env bash

if ! [ -x "$(command -v gmp)" ]; then
  echo 'Error: gmp is not installed.'
  exit 1
elif ! [ -x "$(command -v i386-elf-binutils)" ]; then
  echo 'Error: i386-elf-binutils is not installed.'
  exit 1
elif ! [ -x "$(command -v libmpc)" ]; then
  echo 'Error: libmpc is not installed.'
  exit 1
elif ! [ -x "$(command -v mpfr)" ]; then
  echo 'Error: mpfr is not installed.'
  exit 1
elif ! [ -x "$(command -v wget)" ]; then
  echo 'Error: wget is not installed.'
  exit 1
elif ! [ -x "$(command -v tar)" ]; then
  echo 'Error: tar is not installed.'
  exit 1
else
  wget https://mirrors.nju.edu.cn/gnu/gcc/gcc-8.2.0/gcc-8.2.0.tar.xz
  tar zxvf gcc-8.2.0.tar.xz
  cd gcc-8.2.0
  ./configure \
    --target=i386-elf \
    --without-isl \
    --disable-multilib \
    --disable-nls \
    --disable-werror \
    --without-headers \
    --with-as=i386-elf-as \
    --with-ld=i386-elf-ld \
    --enable-languages=c,c++
  make all-gcc
  make install-gcc
  make all-target-libgcc
  make install-target-libgcc
fi
