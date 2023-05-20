#!/bin/bash

# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# run.sh for Simple-XX/SimpleKernel.
# 在虚拟机中运行内核

# shell 执行出错时终止运行
set -e
# 输出实际执行内容
#set -x

source ./tools/env.sh

# 重新编译
mkdir -p ./build_${ARCH}/
rm -rf ./build_${ARCH}/*
cd ./build_${ARCH}
cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/${TOOLS} -DARCH=${ARCH} -DCMAKE_BUILD_TYPE=DEBUG ..
make
cd ../

# 如果是 riscv 64，需要使用 opensbi
if [ ${ARCH} == "riscv64" ]; then
  # OPENSBI 不存在则编译
  if [ ! -f ${OPENSBI} ]; then
    echo build opensbi.
    cd ./tools/opensbi
    mkdir -p build
    export CROSS_COMPILE=${TOOLCHAIN_PREFIX}
    export FW_JUMP=y
    export FW_JUMP_ADDR=0x80200000
    export PLATFORM_RISCV_XLEN=64
    export PLATFORM=generic
    make
    cd ../..
    echo build opensbi done.
  fi
fi

# 初始化 gdb
if [ ${DEBUG} == 1 ]; then
  cp ./tools/gdbinit ./.gdbinit
  echo "" >>./.gdbinit
  echo "file "${kernel} >>./.gdbinit
  if [ ${ARCH} == "riscv64" ]; then
    echo "add-symbol-file "${OPENSBI} >>./.gdbinit
  fi
  echo "target remote localhost:1234" >>./.gdbinit
  GDB_OPT='-S -gdb tcp::1234'
  echo "Run gdb-multiarch in another shell"
fi

# 运行虚拟机
if [ ${ARCH} == "x86_64" ]; then
  objcopy --target=efi-app-x86-64 ${kernel} kernel.efi \
    -g -R .comment -R .gnu_debuglink -R .note.gnu.build-id \
    -R .gnu.hash -R .plt -R .rela.plt -R .dynstr -R .dynsym -R .rela.dyn \
    -S -R .eh_frame -R .gcc_except_table
  mkdir -p image
  cp *.efi image/
  qemu-system-x86_64 -bios ./3rd/ovmf/OVMF_x86_64.fd -m 128M \
    -monitor telnet::2333,server,nowait -serial stdio \
    -hda fat:rw:image -net none \
    ${GDB_OPT}
elif [ ${ARCH} == "aarch64" ]; then
  qemu-system-aarch64 -machine virt -cpu cortex-a72 -kernel ${kernel} \
    -monitor telnet::2333,server,nowait -serial stdio -nographic \
    ${GDB_OPT}
elif [ ${ARCH} == "riscv64" ]; then
  qemu-system-riscv64 -machine virt -bios ${OPENSBI} -kernel ${kernel} \
    -monitor telnet::2333,server,nowait -serial stdio -nographic \
    ${GDB_OPT}
fi
