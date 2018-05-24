
# This file is apart of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

# setup.sh for MRNIU/SimpleKernel.

# 把 boot.img 挂载到当前目录，然后将 kernel.img 写入 boot 目录，取消挂载。
# 以 bochrc.txt 为配置文件运行 bochs。
cd src/
make
cd ..
hdiutil attach -mountpoint ./boot_folder boot.img
# mac 下的命令与 linux 的不同
# mac's command is different from linux
cp ./src/kernel.kernel boot_folder/boot
hdiutil detach boot_folder
bochs -f bochsrc.txt
