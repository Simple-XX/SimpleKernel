
# This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

# Makefile.env for MRNIU/SimpleKernel.


# 设置 make 环境

CURR_DIR := $(shell pwd)
ROOT_DIR := $(CURR_DIR)
ARCH_DIR := $(ROOT_DIR)/arch/x86_64
BOOT_DIR := $(ARCH_DIR)/boot
DRV_DIR := $(ROOT_DIR)/drv
KERNEL_DIR := $(ROOT_DIR)/kernel
LIBC_DIR := $(ROOT_DIR)/libc

include $(ROOT_DIR)/make.env

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
	$(MAKE) bootloader.bin
	$(MAKE) kernel.bin
	@echo Leave $(CURR_DIR)

.PHONY: remake
remake:
	$(MAKE) clean
	@$(MAKE) all

# 删除源码外的所有文件
.PHONY: clean
clean:
	@echo Entry $(CURR_DIR)
	@echo 正在删除... $<
	@find . -name "*.o"  | xargs rm -f
	@find . -name "*.gch"  | xargs rm -f
	@rm *.bin
	@for subdir in $(SUB_DIR); \
		do $(MAKE) -C $$subdir clean || exit 1; \
	done
	@echo 删除完毕
	@echo Leave $(CURR_DIR)

ifeq ($(ARCH),x86_64)
    KERNEL_C_SOURCES := $(shell find . -path './arch/x86_64/boot' -prune -o -name "*.c" -print)
    KERNEL_CXX_SOURCES := $(shell find . -name "*.cpp")
    KERNEL_S_SOURCES := $(shell find . -path './arch/x86_64/boot' -prune -o -name "*.s" -print)
    KERNEL_OBJ := $(patsubst %.c, %.o, $(KERNEL_C_SOURCES)) \
         		  $(patsubst %.cpp, %.o, $(KERNEL_CXX_SOURCES)) \
	 	    	  $(patsubst %.s, %.o, $(KERNEL_S_SOURCES))

    BOOTLOADER_C_SOURCES := $(shell find ./arch/x86_64/boot -name "*.c")
    BOOTLOADER_CXX_SOURCES := $(shell find ./arch/x86_64/boot -name "*.cpp")
    BOOTLOADER_S_SOURCES := $(shell find ./arch/x86_64/boot -name "*.s")
    BOOTLOADER_OBJ := $(patsubst %.s, %.o, $(BOOTLOADER_S_SOURCES)) \
	    			  $(patsubst %.c, %.o, $(BOOTLOADER_C_SOURCES)) \
		    		  $(patsubst %.cpp, %.o, $(BOOTLOADER_CXX_SOURCES))

    .PHONY: bootloader
    bootloader.bin: LDFLAGS = -A i386 -m elf_i386 -T$(BOOT_DIR)/link32.ld
    bootloader.bin: $(BOOTLOADER_OBJ)
	    @echo 正在生成 BootLoader... $<
    	@$(LD) $(LDFLAGS) $^ -o $@

    .PHONY: kernel.bin
    kernel.bin: $(KERNEL_OBJ)
	    @echo 正在生成内核... $<
        @$(LD) $(LDFLAGS) $^ -o $@
else
    C_SOURCES := $(shell find . -name "*.c")
    CXX_SOURCES := $(shell find . -name "*.cpp")
    S_SOURCES := $(shell find . -name "*.s")
    OBJ := $(patsubst %.c, %.o, $(C_SOURCES)) \
         		  $(patsubst %.cpp, %.o, $(CXX_SOURCES)) \
	 	    	  $(patsubst %.s, %.o, $(S_SOURCES))

    .PHONY: bootloader
    bootloader.bin:
	    @echo i386 不会生成 bootloader.bin

    .PHONY: kernel.bin
    kernel.bin: $(OBJ)
	    @echo 正在生成内核... $<
		@$(LD) $(LDFLAGS) $^ -o $@
endif

.PHONY: code_line_count
code_line_count:
	find . -type f -name "*.[c|cpp|s|S|h|hpp]" -exec cat {} \; | wc -l

.PHONY: generate_map
generate_map:
	$(READELF) -s $(IMAGE) >> $(MAP)

.PHONY: generate_nm
generate_nm:
	$(NM) $(IMAGE)

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
	@echo BOOTLOADER_OBJ: $(BOOTLOADER_OBJ)
	@echo KERNEL_OBJ: $(KERNEL_OBJ)
