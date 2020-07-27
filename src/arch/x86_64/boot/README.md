# /src/arch/i386/boot

## 文件说明

主要负责grub的编写，参考multiboot2编写规范，GRUB的全称是GRand Unified Bootloader，是一个多重操作系统启动管理器，用来引导不同的操作系统。

- include

  bootinit 头文件

- boot.s

  grub的配置文件，从_start地方开始，也就是代码98行

- bootinit.c

  在加载结束grub后，运行bootinit 初始化页表，这里是higher-half kernel ，所以需要进行一系列操作。可参考源码中注释。

- link32.ld

  该脚本告诉ld程序如何构造我们所需32位的内核映像文件。其构建逻辑

- link64.ld

  该脚本告诉ld程序如何构造我们所需64位的内核映像文件。

- Makefile
  


## 参考资料

https://www.gnu.org/software/grub/manual/multiboot/multiboot.html