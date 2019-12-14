
# This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

# Makefile for MRNIU/SimpleKernel.

CURR_DIR := $(shell pwd)
PAR_DIR := $(CURR_DIR)/..
ROOT_DIR := $(CURR_DIR)/../..
include $(ROOT_DIR)/make.env

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
	@$(MAKE) $(patsubst %.c, %.o, $(wildcard *.c))
	@echo Leave $(CURR_DIR)

%.o: %.c
	@echo 编译 C 文件 $< ...
	@$(CC) $(CFLAGS) $< -o $@
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
