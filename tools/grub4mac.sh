
# This file is a part of SimpleXX/SimpleKernel (https://github.com/SimpleXX/SimpleKernel).
#
# grub4mac.sh for SimpleXX/SimpleKernel.

#!/bin/bash

# set -x
set -e

wget https://ftp.gnu.org/gnu/grub/grub-2.04.tar.xz
tar zxvf grub-2.04.tar.xz
cd grub-2.04
./autogen.sh
mkdir -p build
cd build
../configure \
    --target=x86_64-elf \
    --prefix=$(pwd)/grub \
    --disable-werror \
    TARGET_CC=x86_64-elf-gcc \
    TARGET_OBJCOPY=x86_64-elf-objcopy \
    TARGET_STRIP=x86_64-elf-strip \
    TARGET_NM=x86_64-elf-nm \
    TARGET_RANLIB=x86_64-elf-ranlib

sed -i "" "s/FILE_TYPE_NO_DECOMPRESS/GRUB_FILE_TYPE_NO_DECOMPRESS/g" $(pwd)/../grub-core/osdep/generic/blocklist.c

make
make install
