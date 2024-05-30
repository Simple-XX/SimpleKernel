# arch

arch 目录下放置了与架构强相关的内容

如 CPU 寄存器读写、内存相关操作等

对链接脚本做的修改：

- x86_64
    
    在 `x86_64-linux-gnu-ld --verbose` 输出的基础上添加了对齐要求 `ALIGN(0x1000)`

- riscv64

    1. 在 `riscv64-linux-gnu-ld --verbose` 输出的基础上添加了对齐要求 `ALIGN(0x1000)`

    2. 将开始地址设置为 0x80200000

    3. 添加 .boot 段


