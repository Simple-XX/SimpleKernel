
# This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
#
# Makefile for MRNIU/SimpleKernel.

CURR_DIR := $(shell pwd)
PAR_DIR := $(CURR_DIR)/..
ROOT_DIR := $(CURR_DIR)/../../..
include $(ROOT_DIR)/Makefile.env

exclude_dirs := include
subdirs := $(shell find . -maxdepth 1 -type d)
subdirs := $(basename $(patsubst ./%, %, $(subdirs)))
subdirs := $(filter-out $(exclude_dirs), $(subdirs))
SUB_DIR := $(subdirs)

.PHONY: default
default:
	@echo Entry $(CURR_DIR)
	@$(MAKE) all
	@echo Leave $(CURR_DIR)

.PHONY: all
all: $(SUB_DIR)
	@echo Entry $(CURR_DIR)
	@for subdir in $(SUB_DIR); \
    	do $(MAKE) -C $$subdir all || exit 1; \
    done
	@# $(MAKE) info
	@$(MAKE) boot.o
	# @$(MAKE) bootinit.o
	@$(MAKE) $(patsubst %.c, %.o, $(wildcard *.c))
	@echo Leave $(CURR_DIR)

.PHONY: boot.o
boot.o: ASFLAGS = --32 -march=corei7
boot.o: boot.s
	@echo 编译汇编文件 $< ...
	@$(AS) $(ASFLAGS) $< -o $@
	@echo 编译完成

# .PHONY: bootinit.o
# bootinit.o: CFLAGS = -std=gnu11 -ffreestanding -nostdinc -nostdlib -fno-exceptions -nostartfiles -fno-builtin \
# 		 -O2 -Wall -Wextra -Wshadow -Wunreachable-code -Winline -Wmissing-prototypes -Wsign-compare \
# 		 -g -ggdb -C -c \
# 		 -I$(LIBC_DIR) -I$(ARCH_DIR) -I$(DRV_DIR) -I$(KERNEL_DIR) -m32
# bootinit.o: bootinit.c
# 	@echo 编译 C 文件 $< ...
# 	@$(CC) $(CFLAGS) $< -o $@
# 	@echo 编译完成

%.o: %.c
	@echo 编译 C 文件 $< ...
	@$(CC) -std=gnu11 -ffreestanding -nostdinc -nostdlib -fno-exceptions -nostartfiles -fno-builtin \
			 -O2 -Wall -Wextra -Wshadow -Wunreachable-code -Winline -Wmissing-prototypes -Wsign-compare \
			 -g -ggdb -C -c \
			 -I$(LIBC_DIR) -I$(ARCH_DIR) -I$(DRV_DIR) -I$(KERNEL_DIR) -I$(INCLUDE_DIR) -m32 $< -o $@
	@echo 编译完成

.PHONY: clean
clean:
	@echo Entry $(CURR_DIR)
	@echo 正在删除... $<
	@echo 删除完毕
	@echo Leave $(CURR_DIR)

.PHONY: info
info:
	@echo Current dir: $(CURR_DIR)
	@echo ARCH: $(ARCH)
	@echo CC: $(CC)
	@echo CFLAGS: $(CFLAGS)
	@echo CXX: $(CXX)
	@echo CXXFLAGS: $(CXXFLAGS)
	@echo LD: $(LD)
	@echo LDFLAGS: $(LDFLAGS)
	@echo AS: $(AS)
	@echo ASFLAGS: $(ASFLAGS)
	@echo exclude_dirs: $(exclude_dirs)
	@echo SUBDIRS: $(SUB_DIR)
