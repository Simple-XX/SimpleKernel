# 在mac上制作grub2引导镜像

- 安装 grub2 在 mac 上
    见 readme
    
1. 首先用 `dd` 命令创建一个空的镜像文件，大小为 100MB
        
        dd if=/dev/zero of=disk.img count=204800 bs=512

2. 用 `fdisk` 分区

```
fdisk -e disk.img

fdisk: could not open MBR file /usr/standalone/i386/boot0: No such file or directory
The signature for this MBR is invalid.
Would you like to initialize the partition table? [y]
Enter 'help' for information
fdisk:*1> disk
Disk: disk.img	geometry: 812/4/63 [204800 sectors]
Change disk geometry? [n]
fdisk:*1> edit 1
         Starting       Ending
 #: id  cyl  hd sec -  cyl  hd sec [     start -       size]
------------------------------------------------------------------------
 1: 00    0   0   0 -    0   0   0 [         0 -          0] unused
Partition id ('0' to disable)  [0 - FF]: [0] (? for help) 0B
Do you wish to edit in CHS mode? [n]
Partition offset [0 - 204800]: [63] 2047
Partition size [1 - 202753]: [202753]
fdisk:*1> write
Writing MBR at offset 0.
fdisk: 1> quit
```
![屏幕快照 2018-12-27 下午3.05.10](https://lh3.googleusercontent.com/-kasCl4Re1Sc/XCR5vPaGvaI/AAAAAAAAAHA/e7osRuERt6sdPjfV0O6zKQfXFDzvuQBVACHMYCw/I/%255BUNSET%255D)


3. 把引导文件和文件系统独立分出来

        dd if=disk.img of=mbr.img bs=512 count=2047
        dd if=disk.img of=fs.img bs=512 skip=2047
4. 挂载

        hdiutil attach -nomount fs.img

5. 安装文件系统
        
        newfs_msdos -F 32 /dev/diskX

6. 卸载掉 fs.img，将处理好的 两个 img 写入

        hdiutil detach /dev/diskX
        cat mbr.img fs.img > disk.img

7. 挂载，并安装 grub 引导

        hdiutil attach disk.img

        i386-elf-grub-install --modules="part_msdos biosdisk fat multiboot configfile" --root-directory="/Volumes/NO NAME" disk.img

![屏幕快照 2018-12-27 下午3.17.23](https://lh3.googleusercontent.com/-9c9SvfMV468/XCR8l5ca-sI/AAAAAAAAAHY/Afu-Zc8IQLY5f9ouUELcqdp4I4TMkokwQCHMYCw/I/%255BUNSET%255D)





# 参考资料

https://wiki.osdev.org/GRUB_2#Installing_GRUB_2_on_OS_X

