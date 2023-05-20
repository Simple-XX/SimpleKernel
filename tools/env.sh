#!/bin/bash

# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# setup.sh for Simple-XX/SimpleKernel.
# 指定要运行的 ARCH，并设置相关数据

# 指定要编译的目标架构 ARCH: riscv64, x86_64, aarch64
#ARCH="aarch64"
ARCH="x86_64"
#ARCH="riscv64"

DEBUG=0

# 内核映像
kernel='./build_'${ARCH}'/bin/kernel.elf'
# 判断操作系统类型
OS=$(uname -s)
# toolchain
if [ "${OS}" == "Linux" ]; then
  if [ "${ARCH}" == "x86_64" ]; then
    TOOLS="x86_64-x86_64.cmake"
    POSIX_UEFI=$(pwd)/3rd/posix-uefi/build/libuefi_${ARCH}.a
    GNU_EFI=$(pwd)/3rd/gnu-efi/bin/${ARCH}/lib/libefi.a
  elif [ "${ARCH}" == "aarch64" ]; then
    TOOLS="aarch64-x86_64.cmake"
  elif [ "${ARCH}" == "riscv64" ]; then
    TOOLS="toolchain_linux_riscv.cmake"
    TOOLCHAIN_PREFIX=riscv64-linux-gnu-
  fi
  OPENSBI="$(pwd)/3rd/opensbi/build/platform/generic/firmware/fw_jump.elf"
elif [ "${OS}" == "Darwin" ]; then
  if [ "${ARCH}" == "i386" ] || [ "${ARCH}" == "x86_64" ]; then
    TOOLS="toolchain_mac_x86_64.cmake"
  elif [ "${ARCH}" == "aarch64" ]; then
    TOOLS="toolchain_mac_aarch64.cmake"
  elif [ "${ARCH}" == "riscv64" ]; then
    TOOLS="toolchain_mac_riscv.cmake"
    TOOLCHAIN_PREFIX=riscv64-unknown-elf-
  fi
  OPENSBI="$(pwd)/3rd/opensbi/build/platform/generic/firmware/fw_jump.elf"
fi
