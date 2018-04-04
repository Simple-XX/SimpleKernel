# Copy from JamesM's kernel development tutorials: 
# (http://www.jamesmolloy.co.uk/tutorial_html/1.-Environment%20setup.html). 

# This file is apart of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel). 


#!/bin/zsh

#sudo losetup /dev/loop0 floppy.img
# 使用 losetup 将磁盘镜像文件虚拟成块设备
#sudo mount /dev/loop0 /mnt
# 挂载块设备
#sudo cp src/kernel /mnt/kernel
#sudo umount /dev/loop0
#sudo losetup -d /dev/loop0

dd if=/dev/zero of=floppy.img bs=512 count=2880
# 创建空的磁盘镜像文件，这里创建一个 1.44M 的软盘
# bs: 每次写入多少字节 count: 写入多少次
sudo mount  floppy.img /mnt
