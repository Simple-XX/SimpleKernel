
# This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

# Makefile for MRNIU/SimpleKernel.

BOOT_DIR = ./arch/x86_64/boot
LIBC_DIR = ./libc
DRV_DIR = ./drv
KERNEL_DIR = ./kernel
ARCH_DIR = ./arch/x86_64
TARGET = x86_64-elf
# TARGET = i386-elf

C_SOURCES = $(shell find . -name "*.c")
CXX_SOURCES = $(shell find . -name "*.cpp")
S_SOURCES = $(shell find . -name "*.s")
OBJ= $(patsubst %.c, %.o, $(C_SOURCES)) \
     $(patsubst %.cpp, %.o, $(CXX_SOURCES)) \
	 $(patsubst %.s, %.o, $(S_SOURCES))

CXX = $(TARGET)-g++
CXXFLAGS = -std=c++11 -ffreestanding -nostdinc -nostdlib -fno-exceptions -fno-rtti -fno-builtin \
 				 -O2 -Wall -Wextra -Wshadow -Wunreachable-code -Winline -Wsign-compare \
				-g -ggdb -C -c \
				-I$(LIBC_DIR) -I$(ARCH_DIR) -I$(DRV_DIR) -I$(KERNEL_DIR)  # -v

ifeq ($(TARGET), "x86_64-elf")
	CXXFLAGS += -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2
endif

OBJCPY = $(TARGET)-objcopy
STRIP = $(TARGET)-strip
NM = $(TARGET)-gcc-nm
READELF = $(TARGET)-readelf

CC = $(TARGET)-gcc
CFLAGS = -std=gnu11 -ffreestanding -nostdinc -nostdlib -fno-exceptions -nostartfiles -fno-builtin \
		 -O2 -Wall -Wextra -Wshadow -Wunreachable-code -Winline -Wmissing-prototypes -Wsign-compare \
		 -g -ggdb -C -c \
		 -I$(LIBC_DIR) -I$(ARCH_DIR) -I$(DRV_DIR) -I$(KERNEL_DIR)  # -v

ifeq ($(TARGET), "x86_64-elf")
	CFLAGS += -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2
endif


LD = $(TARGET)-ld
LDFLAGS = -A x86_64 -m elf_x86_64 -T$(BOOT_DIR)/link.ld

AS = $(TARGET)-as
ASFLAGS = -march=corei7

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
	find . -type f -name "*.[c|cpp|s|h|hpp]" -exec cat {} \; | wc -l

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
