
# This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

# Makefile for MRNIU/SimpleKernel.

BOOT_DIR= ./arch/i386/boot
HPP_DIR= ./include
LIBC_DIR= ./include/libc
ARCH_DIR= ./arch/i386

C_SOURCES = $(shell find . -name "*.c")
S_SOURCES = $(shell find . -name "*.s")
OBJ= $(patsubst %.c, %.o, $(C_SOURCES)) \
     $(patsubst %.s, %.o, $(S_SOURCES))


CC= i386-elf-gcc
CFLAGS= -std=gnu99 -ffreestanding -nostdinc -O2 -Wall -Wextra -c -I$(LIBC_DIR)\
 				-I$(HPP_DIR) -I$(ARCH_DIR) -I$(ARCH_DIR)/include

LD= i386-elf-ld
LDFLAGS= -A elf32_i386 -T$(BOOT_DIR)/link.ld

AS= i386-elf-as
ASFLAGS= -march=i386

# 生成 kernel
kernel.kernel: $(OBJ)
	@echo 正在生成内核... $<
	$(LD) $(LDFLAGS) $^ -o $@

.s.o:
	@echo 编译汇编文件... $<
	$(AS) $(ASFLAGS) $< -o $@

.c.o:
	@echo 编译C文件... $<
	$(CC) $(CFLAGS) $< -o $@

# 删除源码外的所有文件
clean:
	@echo 正在删除... $<
	find . -name "*.o"  | xargs rm -f
	-rm kernel.kernel
