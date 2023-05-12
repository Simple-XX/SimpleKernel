#!/bin/bash

# This file is a part of Simple-XX/SimpleKernel
# (https://github.com/Simple-XX/SimpleKernel).
#
# mkfatfs.sh for Simple-XX/SimpleKernel.

# 制作 fatfs 镜像

# dst = /mnt  space aside "=" is wrong!!!
dst=/mnt
# echo $dst
dd if=/dev/zero of=fatfs.img bs=512k count=96
# echo $dst
mkfs.vfat -F 32 fatfs.img
# echo $dst
sudo mount fatfs.img $dst
# echo $dst
# echo hi
cd $dst
sudo mkdir dir1
# pwd
sudo mkdir dir2
sudo mkdir dir3
sudo dash -c "echo hello world > file1"
cd dir1
sudo mkdir dir4
sudo sh -c "echo hi my bro! > file2"
cd dir4
sudo sh -c "echo yes my girl! > file3"
cd /
sudo umount $dst
