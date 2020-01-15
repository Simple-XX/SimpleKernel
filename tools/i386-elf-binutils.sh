
# This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
#
# i386-elf-binutils.sh for MRNIU/SimpleKernel.

#!/usr/bin/env bash

if ! [ -x "$(command -v wget)" ]; then
  echo 'Error: wget is not installed.'
  exit 1
elif ! [ -x "$(command -v tar)" ]; then
  echo 'Error: tar is not installed.'
  exit 1
else
  wget https://ftp.gnu.org/gnu/binutils/binutils-2.31.1.tar.xz
  tar xvJf binutils-2.31.1.tar.xz
  cd binutils-2.31.1
  ./configure \
    --target=i386-elf \
    --disable-multilib \
    --disable-nls \
    --disable-werror
  make
  make install
fi
