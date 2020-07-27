# /src/arch/x86_64/task

对进程管理，通过进程控制块（pcb）的操作。

## 文件说明

- arch_init.c

  初始化硬件相关配置，包括gdt和idt。

- cpu.hpp

  部分CPU变量宏定义以及辅助函数。

- port.hpp

  端口读写操作函数。

- sync.hpp

  中断配置。

## 参考资料

http://wiki.0xffffff.org/posts/hurlex-12.html

