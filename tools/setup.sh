
# This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
#
# setup.sh for MRNIU/SimpleKernel.

#!/usr/bin/env bash
# shell 执行出错时终止运行
# set -e
# 输出实际执行内容
# set -x

source ./env.sh

# 检测环境，如果没有安装需要的软件，则安装
if ! [ -x "$(command -v ${SIMULATOR})" ]; then
    echo 'Error: '${SIMULATOR}' is not installed.'
    echo 'Install '${SIMULATOR}'...'
    if [ ${OS} == "Darwin" ]; then
        brew install ${SIMULATOR}
    elif [ ${OS} == "Linux" ]; then
        shell ${tool}/${SIMULATOR}.sh
    fi
fi

if ! [ -x "$(command -v ${TARGET}-ld)" ]; then
    echo 'Error: '${TARGET}'-binutils is not installed.'
    echo 'Install '${TARGET}'-binutils...'
    if [ ${OS} == "Darwin" ]; then
        brew install ${TARGET}-binutils
    elif [ ${OS} == "Linux" ]; then
        shell ${tool}/${TARGET}-binutils.sh
    fi
fi

if ! [ -x "$(command -v ${TARGET}-gcc)" ]; then
    echo 'Error: '${TARGET}'-gcc is not installed.'
    echo 'Install '${TARGET}'-gcc...'
    if [ ${OS} == "Darwin" ]; then
        brew install ${TARGET}-gcc
    elif [ ${OS} == "Linux" ]; then
        shell ${tool}/${TARGET}-gcc.sh
    fi
fi

if [ ${OS} == "Darwin" ]; then
    if ! [ -x "$(command -v i386-elf-grub-file)" ]; then
        echo 'Error: i386-elf-grub is not installed.'
        echo 'Install i386-elf-grub...'
        brew install i386-elf-grub
    fi
elif [ ${OS} == "Linux" ]; then
    if ! [ -x "$(command -v grub-file)" ]; then
        shell ${tool}/grub.sh
    fi
fi

if ! [ -x "$(command -v i386-elf-grub-file)" ]; then
    echo 'Error: i386-elf-grub is not installed.'
    echo 'Install i386-elf-grub...'
    if [ ${OS} == "Darwin" ]; then
        brew install i386-elf-grub
    elif [ ${OS} == "Linux" ]; then
        shell ${tool}/i386-elf-grub.sh
    fi
fi

if ! [ -x "$(command -v xorriso)" ]; then
    echo 'Error: xorriso is not installed.'
    echo 'Install xorriso...'
    if [ ${OS} == "Darwin" ]; then
        brew install xorriso
    elif [ ${OS} == "Linux" ]; then
        shell ${tool}/xorriso.sh
    fi
fi
