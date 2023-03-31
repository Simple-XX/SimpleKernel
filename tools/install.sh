#!/bin/bash

# (https://github.com/Simple-XX/SimpleKernel).
#
# run.sh for Simple-XX/SimpleKernel.
# 在虚拟机中运行内核

# shell 执行出错时终止运行
set -e
# 输出实际执行内容
# set -x

# 命令行使用方式
help() {
    echo "Usage:"
    echo "run.sh [-t target arch] [-s simulator]"
    echo "Description:"
    echo "-t, 'riscv64', 'i386', 'x86_64', 'arm', 'aarch64'. riscv64 as default"
    echo "-s, 'qemu', 'bochs'(i386/x86_64 only). qemu as default"
    exit 1
}

# 如果没有传入参数或参数过多则退出
if [ $# == 0 ] || [ $# -gt 4 ]; then
    help
    exit 1
fi

# 解析参数
while getopts 't:s:h' opt; do
    case $opt in
        t)
            TARGET="$OPTARG"
            ;;
        s)
            SIMULATOR="$OPTARG"
            ;;
        h)
            help
            ;;
        ?)
            echo "missing  options,pls check!"
            exit 1
            ;;
    esac
done

# 如果没有确定必要参数则报错
# 参数不为空
if [ -z "$TARGET" ] || [ -z "$SIMULATOR" ]; then
    help
    exit 1
fi

# 判断是否合法
# 目标架构
targets=(
riscv64
i386
x86_64
arm
aarch64
)
if [[ ! "${targets[*]}"  =~ $TARGET ]]; then
    echo "$TARGET not exists"
    help
    exit 1
fi
# 模拟器
simulators=(
qemu
bochs
)
if [[ ! "${simulators[*]}"  =~ $SIMULATOR ]]; then
    echo "$SIMULATOR not exists"
    help
    exit 1
fi
# bochs 仅用于 i386/x86_64
if [ "$SIMULATOR" == "bochs" ] && [ "$TARGET" != "i386" ] && [ "$TARGET" != "x86_64" ]; then
    echo "bochs is i386/x86_64 only."
    help
    exit 1
fi

# 宿主机器架构
HOST_ARCH=$(uname -m)
# 宿主机器系统
HOST_SYSTEM=$(uname -s)

# 输出信息
echo "host arch:        $HOST_ARCH"
echo "host system:      $HOST_SYSTEM"
echo "target arch:      $TARGET"
echo "target simulator: $SIMULATOR"

# 根据参数与机器信息确认依赖
# 宿主机器为 x86_64
if [ "${HOST_ARCH}" == "x86_64" ]; then
    # 宿主机器为 linux
    if [ "${HOST_SYSTEM}" == "Linux" ]; then
        # target 为 riscv64
        if [ "${TARGET}" == "riscv64" ]; then
            echo 1
        # target 为 arm/aarch64
        elif [ "${TARGET}" == "arm" ] || [ "${TARGET}" == "aarch64" ]; then
            echo 1
        # target 为 i386/x86_64
        elif [ "${TARGET}" == "i386" ] || [ "${TARGET}" == "x86_64" ]; then
            # 模拟器为 bochs
            if [ "${SIMULATOR}" == "bochs" ]; then
                echo 1
            fi
        fi
    # 宿主机器为 macos
    elif [ "${HOST_SYSTEM}" == "Darwin" ]; then
        # target 为 riscv64
        if [ "${TARGET}" == "riscv64" ]; then
            echo 1
        # target 为 arm/aarch64
        elif [ "${TARGET}" == "arm" ] || [ "${TARGET}" == "aarch64" ]; then
            echo 1
        # target 为 i386/x86_64
        elif [ "${TARGET}" == "i386" ] || [ "${TARGET}" == "x86_64" ]; then
            # 模拟器为 bochs
            if [ "${SIMULATOR}" == "bochs" ]; then
                echo 1
            fi
        fi
    fi
# 宿主机器为 arm64
elif [ "${HOST_ARCH}" == "arm64" ]; then
    # 宿主机器为 linux
    if [ "${HOST_SYSTEM}" == "Linux" ]; then
        # target 为 riscv64
        if [ "${TARGET}" == "riscv64" ]; then
            echo 1
        # target 为 arm/aarch64
        elif [ "${TARGET}" == "arm" ] || [ "${TARGET}" == "aarch64" ]; then
            echo 1
        # target 为 i386/x86_64
        elif [ "${TARGET}" == "i386" ] || [ "${TARGET}" == "x86_64" ]; then
            # 模拟器为 bochs
            if [ "${SIMULATOR}" == "bochs" ]; then
                echo 1
            fi
        fi
    # 宿主机器为 macos
    elif [ "${HOST_SYSTEM}" == "Darwin" ]; then
        # 安装通用依赖
        # make
        if  ! command -v make &> /dev/null; then
            echo "make could not found."
            echo "Installing make..."
            # 安装 make
            brew install make
            echo "make installed."
        fi
        # cmake
        if  ! command -v cmake &> /dev/null; then
            echo "cmake could not found."
            echo "Installing cmake..."
            # 安装 cmake
            brew install cmake
            echo "cmake installed."
        fi
        # qemu
        if  ! command -v qemu-system-riscv64 &> /dev/null; then
            echo "qemu could not found."
            echo "Installing qemu..."
            # 安装 qemu
            brew install qemu
            echo "qemu installed."
        fi
        # target 为 riscv64
        if [ "${TARGET}" == "riscv64" ]; then
            # riscv64-unknown-elf-gcc
            if  ! command -v riscv64-unknown-elf-gcc &> /dev/null; then
                echo "riscv64-unknown-elf-gcc could not found."
                echo "Installing riscv64-unknown-elf-gcc..."
                # 安装 riscv64-unknown-elf-gcc
                brew tap riscv-software-src/riscv
                brew install riscv-software-src/riscv/riscv-gnu-toolchain
                echo "riscv64-unknown-elf-gcc installed."
            fi
        # target 为 arm/aarch64
        elif [ "${TARGET}" == "arm" ] || [ "${TARGET}" == "aarch64" ]; then
            echo 1
        # target 为 i386/x86_64
        elif [ "${TARGET}" == "i386" ] || [ "${TARGET}" == "x86_64" ]; then
            # x86_64-elf-gcc
            if  ! command -v x86_64-elf-gcc &> /dev/null; then
                echo "x86_64-elf-gcc could not found."
                echo "Installing x86_64-elf-gcc..."
                # 安装 x86_64-elf-gcc
                brew install x86_64-elf-gcc
                echo "x86_64-elf-gcc installed."
            fi
            # 模拟器为 bochs
            if [ "${SIMULATOR}" == "bochs" ]; then
                # bochs
                if  ! command -v bochs &> /dev/null; then
                    echo "bochs could not found."
                    echo "Installing bochs..."
                    # 安装 bochs
                    brew install bochs
                    echo "bochs installed."
                fi
            fi
        fi
    fi
fi
