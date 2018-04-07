# Copy from JamesM's kernel development tutorials:
# (http://www.jamesmolloy.co.uk/tutorial_html/1.-Environment%20setup.html).

# This file is apart of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

#!/bin/bash

# run_bochs.sh
# mounts the correct loopback device, runs bochs, then unmounts.


hdiutil attach -mountpoint ./boot_folder boot.img
# mac 下的命令与 linux 的不同
# mac's command is different from linux
cp ./src/kernel boot_folder/boot
hdiutil detach boot_folder
bochs -f bochsrc.txt
