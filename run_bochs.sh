# Copy from JamesM's kernel development tutorials: 
# (http://www.jamesmolloy.co.uk/tutorial_html/1.-Environment%20setup.html). 

# This file is apart of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel). 

#!/bin/bash

# run_bochs.sh
# mounts the correct loopback device, runs bochs, then unmounts.

sudo /sbin/losetup /dev/loop0 floppy.img
sudo bochs -f bochsrc.txt
sudo /sbin/losetup -d /dev/loop0
