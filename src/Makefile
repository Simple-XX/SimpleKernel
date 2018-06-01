
# This file is apart of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

# Makefile for MRNIU/SimpleKernel.

BOOT_DIR= ./arch/i386
KERNEL_DIR= ./kernel
LIBC_DIR= ./include/libc

OBJ= $(BOOT_DIR)/boot.o $(KERNEL_DIR)/kernel.o $(LIBC_DIR)/stdio/vsprintf.o \
     $(KERNEL_DIR)]/printk.o
CC= i386-elf-gcc
CFLAGS= -std=gnu99 -ffreestanding -O2 -Wall -Wextra -c
# -ffreestanding 按独立环境编译,允许指定程序入口.
# 详情参考gcc网站最近的资料

LD= i386-elf-ld
LDFLAGS= -A elf32_i386 -T$(BOOT_DIR)/link.ld
AS= i386-elf-as
ASFLAGS= -march=i386

# 生成 ketnel
kernel.kernel: $(OBJ)
	$(LD) $(LDFLAGS) $^ -o $@

# 生成 main.o
kernel.o: $(KERNEL_DIR)/kernel.c
	$(CC) $(CFLAGS) $^ -o $@

vsprintf.o: $(LIBC_DIR)/stdio/vsprintf.c
	$(CC) $(CFLAGS) $^ -o $@

printk.o: $(KERNEL_DIR)/printk.c
	$(CC) $(CFLAGS) $^ -o $@

# 生成 boot.o
boot.o: $(BOOT_DIR)/boot.s
	$(AS) $(ASFLAGS) $^ -o $@

# 删除源码外的所有文件
clean:
	find . -name "*.o"  | xargs rm -f
	-rm kernel.kernel
