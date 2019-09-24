
# This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

# Makefile for MRNIU/SimpleKernel.

BOOT_DIR = ./arch/x86_64/boot
LIBC_DIR = ./libc
DRV_DIR = ./drv
KERNEL_DIR = ./kernel
ARCH_DIR = ./arch/x86_64

C_SOURCES = $(shell find . -name "*.c")
S_SOURCES = $(shell find . -name "*.s")
OBJ= $(patsubst %.c, %.o, $(C_SOURCES)) \
     $(patsubst %.s, %.o, $(S_SOURCES)) \
	 $(patsubst %.cpp, %.o, $(CXX_SOURCES))

CXX = i386-elf-g++
CXXFLAGS = -std=c++11 -ffreestanding -nostdinc -O2 \
 				-Wall -Wextra -Wshadow -Wunreachable-code -Winline -Wsign-compare \
				-g -ggdb -C -c \
				-I$(LIBC_DIR) -I$(ARCH_DIR) -I$(DRV_DIR) -I$(KERNEL_DIR)  # -v

OBJCPY = i386-elf-objcopy
STRIP = i386-elf-strip
NM = i386-elf-gcc-nm
READELF = i386-elf-readelf

CC = i386-elf-gcc
CFLAGS = -std=gnu11 -ffreestanding -nostdinc -O2 \
		 -Wall -Wextra -Wshadow -Wunreachable-code -Winline -Wmissing-prototypes -Wsign-compare \
		 -g -ggdb -C -c \
		 -I$(LIBC_DIR) -I$(ARCH_DIR) -I$(DRV_DIR) -I$(KERNEL_DIR)  # -v

LD = i386-elf-ld
LDFLAGS = -A elf32_i386 -T$(BOOT_DIR)/link.ld

AS = i386-elf-as
ASFLAGS = -march=i386

IMAGE = kernel.kernel
MAP = kernel.map

# 生成 kernel
kernel.kernel: $(OBJ)
	@echo 正在生成内核... $<
	@$(LD) $(LDFLAGS) $^ -o $@

.c.o:
	@echo 编译C文件... $<
	@$(CC) $(CFLAGS) $< -o $@

.s.o:
	@echo 编译汇编文件... $<
	@$(AS) $(ASFLAGS) $< -o $@

default:
	make kernel.kernel

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

.PHONY: code_line_count
code_line_count:
	find . -type f -name "*.[c|h|s|hpp]" -exec cat {} \; | wc -l

.PHONY: generate_map
generate_map:
	$(READELF) -s $(IMAGE) >> $(MAP)

.PHONY: generate_nm
generate_nm:
	$(NM) $(IMAGE)

.PHONY: remake
remake:
	$(MAKE) clean
	$(MAKE) default

# 删除源码外的所有文件
.PHONY: clean
clean:
	@echo 正在删除... $<
	@-rm -f $(IMAGE)
	@-rm -f $(MAP)
	@find . -name "*.o"  | xargs rm -f
	@find . -name "*.gch"  | xargs rm -f
	@echo 删除完毕
