# /src/arch/x86_64/gdt

## 文件说明

对全局描述表（Global Describe Table）进行处理配置，这部分代码主要是用于系统初始化过程。

- include

  gdt头文件

- gdt_s.s

  定义两个函数gdt_load和tss_load 用于加载加载全局描述符表地址到 GDTR 寄存器以及加载任务寄存器

- gdt.c 

  定义和初始化全局描述符表

- Makefile
  


## 参考资料

http://wiki.0xffffff.org/posts/hurlex-6.html