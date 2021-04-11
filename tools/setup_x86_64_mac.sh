
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# setup_x86_64_mac.sh for Simple-XX/SimpleKernel.

#!/bin/bash

# shell 执行出错时终止运行
set -e
# 输出实际执行内容
set -x

source ./env.sh

# 检测环境，如果没有安装需要的软件，则安装
if ! [ -x "$(command -v ${SIMULATOR})" ]; then
    echo 'Error: '${SIMULATOR}' is not installed.'
    echo 'Install '${SIMULATOR}'...'
    brew install ${SIMULATOR}
fi

if ! [ -x "$(command -v x86_64-elf-ld)" ]; then
    echo 'Error: x86_64-elf-binutils is not installed.'
    echo 'Install x86_64-elf-binutils...'
    brew install x86_64-elf-binutils
fi

if ! [ -x "$(command -v x86_64-elf-gcc)" ]; then
    echo 'Error: x86_64-elf-gcc is not installed.'
    echo 'Install x86_64-elf-gcc...'
    brew install x86_64-elf-gcc
fi

if ! [ -x "$(command -v xorriso)" ]; then
    echo 'Error: xorriso is not installed.'
    echo 'Install xorriso...'
    brew install xorriso
fi

if ! [ -x "$(command -v $(GRUB_PATH)/grub-file)" ]; then
    sh ./grub4mac.sh
fi
