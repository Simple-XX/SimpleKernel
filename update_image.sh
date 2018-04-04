# Copy from JamesM's kernel development tutorials: 
# (http://www.jamesmolloy.co.uk/tutorial_html/1.-Environment%20setup.html). 

# This file is apart of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel). 


#!/bin/bash

sudo losetup /dev/loop0 floppy.img
sudo mount /dev/loop0 /mnt
sudo cp src/kernel /mnt/kernel
sudo umount /dev/loop0
sudo losetup -d /dev/loop0
