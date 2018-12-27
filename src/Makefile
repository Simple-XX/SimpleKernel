
# This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

# Makefile for MRNIU/SimpleKernel.

BOOT_DIR= ./arch/i386/boot
HPP_DIR= ./include
LIBC_DIR= ./include/libc
ARCH_DIR= ./arch/i386

C_SOURCES = $(shell find . -name "*.c")
# CXX_SOURCES = $(shell find ./include/DataStructuer/ -name "*.cpp")
S_SOURCES = $(shell find . -name "*.s")
OBJ= $(patsubst %.c, %.o, $(C_SOURCES)) \
     $(patsubst %.s, %.o, $(S_SOURCES)) \
	   $(patsubst %.cpp, %.o, $(CXX_SOURCES))


CXX= i386-elf-g++
CXXFLAGS= -std=c++11 -ffreestanding -nostdinc -O2 \
 				-Wall -Wextra -Wshadow -Wunreachable-code -Winline -Wsign-compare \
				-g -ggdb -C -c \
				-I$(LIBC_DIR) -I$(HPP_DIR) -I$(ARCH_DIR) -I$(ARCH_DIR)/include  # -v

CC= i386-elf-gcc
CFLAGS= -std=gnu99 -ffreestanding -nostdinc -O2 \
 				-Wall -Wextra -Wshadow -Wunreachable-code -Winline -Wmissing-prototypes -Wsign-compare \
				-g -ggdb -C -c \
				-I$(LIBC_DIR) -I$(HPP_DIR) -I$(ARCH_DIR) -I$(ARCH_DIR)/include  # -v

LD= i386-elf-ld
LDFLAGS= -A elf32_i386 -T$(BOOT_DIR)/link.ld

AS= i386-elf-as
ASFLAGS= -march=i386

# 生成 kernel
kernel.kernel: $(OBJ)
	@echo 正在生成内核... $<
	@$(LD) $(LDFLAGS) $^ -o $@

.c.o:
	@echo 编译C文件... $<
	@$(CC) $(CFLAGS) $< -o $@

.cpp.o:
	@echo 编译C++文件... $<
	@$(CXX) $(CXXFLAGS) $< -o $@

.s.o:
	@echo 编译汇编文件... $<
	@$(AS) $(ASFLAGS) $< -o $@

.PHONY: info
info:
	@echo CC: $(CC)
	@echo CFLAGS: $(CFLAGS)
	@echo CXX: $(CXX)
	@echo CXXFLAGS: $(CXXFLAGS)
	@echo LD: $(LD)
	@echo LDFLAGS: $(LDFLAGS)
	@echo AS: $(AS)
	@echo ASFLAGS: $(ASFLAGS)

.PHONY:code_line_count
code_line_count:
	find . -type f -name "*.[c|h|s|cpp|hpp]" -exec cat {} \; | wc -l

# 删除源码外的所有文件
.PHONY: clean
clean:
	@echo 正在删除... $<
	@find . -name "*.o"  | xargs rm -f
	@-rm -f kernel.kernel
	@echo 删除完毕
