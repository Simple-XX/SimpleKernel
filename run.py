
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# run.py for Simple-XX/SimpleKernel.

#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import argparse
import subprocess

# 通用依赖
# cmake, make
# i386/x86_64 需要的依赖
# bochs, grub2, x86_64-elf-g++
# arm 需要的依赖
# qemu-system-aarch64,  arm-none-eabi-g++
# riscv64 需要的依赖
# qemu-system-riscv64, riscv64-unknown-elf-g++
# Depends = ["cmake", "make"]

# 仅运行，默认所有依赖都可用


# 清空旧目录
def clean():
    proc = subprocess.call(
        "rm -rf ./build/*", shell=True)
    proc = subprocess.call(
        "mkdir -p ./build/", shell=True)
    return


# 执行 CMake
def cmake(arch):
    if arch == "riscv" or arch == "riscv64":
        proc = subprocess.call(
            ["cd ./build && cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/toolchain_mac_riscv.cmake -DARCH=riscv64 -DCMAKE_BUILD_TYPE=DEBUG .."], shell=True)
    return


def run(arch, simulator):
    if arch == "riscv" or arch == "riscv64":
        proc = subprocess.call(
            "qemu-system-riscv64 -machine virt -serial stdio -bios ./tools/opensbi/build/platform/generic/firmware/fw_jump.elf -kernel ./build/bin/kernel.elf", shell=True)
    return


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-v", "--version", help="Print version.", action='version', version='dev')
    # 目标平台
    parser.add_argument("-a", "--arch", required=False, default="i386", type=str,
                        help="i386, x86_64, arm, aarch64, riscv64")
    # 虚拟机
    parser.add_argument("-s", "--simulator", required=False, default="bochs", type=str,
                        help="bochs, qemu")
    args = parser.parse_args()
    # i386/x86_64 暂不支持 qemu 模拟
    if (args.arch == "i386" or args.arch == "x86_64") and args.simulator == "qemu":
        print("Not Support yet.")
        exit(1)
    clean()
    cmake(args.arch)
    # 执行 make
    proc = subprocess.call(
        ["cd ./build && make"], shell=True)
    run(args.arch, args.simulator)
    return


if __name__ == '__main__':
    main()
