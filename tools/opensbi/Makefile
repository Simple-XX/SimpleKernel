#
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2019 Western Digital Corporation or its affiliates.
#
# Authors:
#   Anup Patel <anup.patel@wdc.com>
#

# Select Make Options:
# o  Do not use make's built-in rules
# o  Do not print "Entering directory ...";
MAKEFLAGS += -r --no-print-directory

# Readlink -f requires GNU readlink
ifeq ($(shell uname -s),Darwin)
READLINK ?= greadlink
else
READLINK ?= readlink
endif

# Find out source, build, and install directories
src_dir=$(CURDIR)
ifdef O
 build_dir=$(shell $(READLINK) -f $(O))
else
 build_dir=$(CURDIR)/build
endif
ifeq ($(build_dir),$(CURDIR))
$(error Build directory is same as source directory.)
endif
install_root_dir_default=$(CURDIR)/install
ifdef I
 install_root_dir=$(shell $(READLINK) -f $(I))
else
 install_root_dir=$(install_root_dir_default)/usr
endif
ifeq ($(install_root_dir),$(CURDIR))
$(error Install root directory is same as source directory.)
endif
ifeq ($(install_root_dir),$(build_dir))
$(error Install root directory is same as build directory.)
endif
ifdef PLATFORM_DIR
  platform_dir_path=$(shell $(READLINK) -f $(PLATFORM_DIR))
  ifdef PLATFORM
    platform_parent_dir=$(platform_dir_path)
  else
    PLATFORM=$(shell basename $(platform_dir_path))
    platform_parent_dir=$(shell realpath ${platform_dir_path}/..)
  endif
else
 platform_parent_dir=$(src_dir)/platform
endif
ifndef PLATFORM_DEFCONFIG
PLATFORM_DEFCONFIG=defconfig
endif

# Check if verbosity is ON for build process
CMD_PREFIX_DEFAULT := @
ifeq ($(V), 1)
	CMD_PREFIX :=
else
	CMD_PREFIX := $(CMD_PREFIX_DEFAULT)
endif

# Setup path of directories
export platform_subdir=$(PLATFORM)
export platform_src_dir=$(platform_parent_dir)/$(platform_subdir)
export platform_build_dir=$(build_dir)/platform/$(platform_subdir)
export include_dir=$(CURDIR)/include
export libsbi_dir=$(CURDIR)/lib/sbi
export libsbiutils_dir=$(CURDIR)/lib/utils
export firmware_dir=$(CURDIR)/firmware

# Setup variables for kconfig
ifdef PLATFORM
export PYTHONDONTWRITEBYTECODE=1
export KCONFIG_DIR=$(platform_build_dir)/kconfig
export KCONFIG_AUTOLIST=$(KCONFIG_DIR)/auto.list
export KCONFIG_AUTOHEADER=$(KCONFIG_DIR)/autoconf.h
export KCONFIG_AUTOCMD=$(KCONFIG_DIR)/auto.conf.cmd
export KCONFIG_CONFIG=$(KCONFIG_DIR)/.config
# Additional exports for include paths in Kconfig files
export OPENSBI_SRC_DIR=$(src_dir)
export OPENSBI_PLATFORM=$(PLATFORM)
export OPENSBI_PLATFORM_SRC_DIR=$(platform_src_dir)
endif

# Find library version
OPENSBI_VERSION_MAJOR=`grep "define OPENSBI_VERSION_MAJOR" $(include_dir)/sbi/sbi_version.h | sed 's/.*MAJOR.*\([0-9][0-9]*\)/\1/'`
OPENSBI_VERSION_MINOR=`grep "define OPENSBI_VERSION_MINOR" $(include_dir)/sbi/sbi_version.h | sed 's/.*MINOR.*\([0-9][0-9]*\)/\1/'`
OPENSBI_VERSION_GIT=$(shell if [ -d $(src_dir)/.git ]; then git describe 2> /dev/null; fi)

# Setup compilation commands
ifneq ($(LLVM),)
CC		=	clang
AR		=	llvm-ar
LD		=	ld.lld
OBJCOPY		=	llvm-objcopy
else
ifdef CROSS_COMPILE
CC		=	$(CROSS_COMPILE)gcc
AR		=	$(CROSS_COMPILE)ar
LD		=	$(CROSS_COMPILE)ld
OBJCOPY		=	$(CROSS_COMPILE)objcopy
else
CC		?=	gcc
AR		?=	ar
LD		?=	ld
OBJCOPY		?=	objcopy
endif
endif
CPP		=	$(CC) -E
AS		=	$(CC)
DTC		=	dtc

ifneq ($(shell $(CC) --version 2>&1 | head -n 1 | grep clang),)
CC_IS_CLANG	=	y
else
CC_IS_CLANG	=	n
endif

ifneq ($(shell $(LD) --version 2>&1 | head -n 1 | grep LLD),)
LD_IS_LLD	=	y
else
LD_IS_LLD	=	n
endif

ifeq ($(CC_IS_CLANG),y)
ifneq ($(CROSS_COMPILE),)
CLANG_TARGET	=	--target=$(notdir $(CROSS_COMPILE:%-=%))
endif
endif

# Guess the compiler's XLEN
OPENSBI_CC_XLEN := $(shell TMP=`$(CC) $(CLANG_TARGET) -dumpmachine | sed 's/riscv\([0-9][0-9]\).*/\1/'`; echo $${TMP})

# Guess the compiler's ABI and ISA
ifneq ($(CC_IS_CLANG),y)
OPENSBI_CC_ABI := $(shell TMP=`$(CC) -v 2>&1 | sed -n 's/.*\(with\-abi=\([a-zA-Z0-9]*\)\).*/\2/p'`; echo $${TMP})
OPENSBI_CC_ISA := $(shell TMP=`$(CC) -v 2>&1 | sed -n 's/.*\(with\-arch=\([a-zA-Z0-9]*\)\).*/\2/p'`; echo $${TMP})
endif

# Setup platform XLEN
ifndef PLATFORM_RISCV_XLEN
  ifeq ($(OPENSBI_CC_XLEN), 32)
    PLATFORM_RISCV_XLEN = 32
  else
    PLATFORM_RISCV_XLEN = 64
  endif
endif

ifeq ($(CC_IS_CLANG),y)
ifeq ($(CROSS_COMPILE),)
CLANG_TARGET	=	--target=riscv$(PLATFORM_RISCV_XLEN)-unknown-elf
endif
endif

ifeq ($(LD_IS_LLD),y)
RELAX_FLAG	=	-mno-relax
USE_LD_FLAG	=	-fuse-ld=lld
else
USE_LD_FLAG	=	-fuse-ld=bfd
endif

# Check whether the linker supports creating PIEs
OPENSBI_LD_PIE := $(shell $(CC) $(CLANG_TARGET) $(RELAX_FLAG) $(USE_LD_FLAG) -fPIE -nostdlib -Wl,-pie -x c /dev/null -o /dev/null >/dev/null 2>&1 && echo y || echo n)

# Check whether the compiler supports -m(no-)save-restore
CC_SUPPORT_SAVE_RESTORE := $(shell $(CC) $(CLANG_TARGET) $(RELAX_FLAG) -nostdlib -mno-save-restore -x c /dev/null -o /dev/null 2>&1 | grep "\-save\-restore" >/dev/null && echo n || echo y)

# Check whether the assembler and the compiler support the Zicsr and Zifencei extensions
CC_SUPPORT_ZICSR_ZIFENCEI := $(shell $(CC) $(CLANG_TARGET) $(RELAX_FLAG) -nostdlib -march=rv$(OPENSBI_CC_XLEN)imafd_zicsr_zifencei -x c /dev/null -o /dev/null 2>&1 | grep "zicsr\|zifencei" > /dev/null && echo n || echo y)

# Build Info:
# OPENSBI_BUILD_TIME_STAMP -- the compilation time stamp
# OPENSBI_BUILD_COMPILER_VERSION -- the compiler version info
BUILD_INFO ?= n
ifeq ($(BUILD_INFO),y)
OPENSBI_BUILD_DATE_FMT = +%Y-%m-%d %H:%M:%S %z
ifdef SOURCE_DATE_EPOCH
	OPENSBI_BUILD_TIME_STAMP ?= $(shell date -u -d "@$(SOURCE_DATE_EPOCH)" \
		"$(OPENSBI_BUILD_DATE_FMT)" 2>/dev/null || \
		date -u -r "$(SOURCE_DATE_EPOCH)" \
		"$(OPENSBI_BUILD_DATE_FMT)" 2>/dev/null || \
		date -u "$(OPENSBI_BUILD_DATE_FMT)")
else
	OPENSBI_BUILD_TIME_STAMP ?= $(shell date "$(OPENSBI_BUILD_DATE_FMT)")
endif
OPENSBI_BUILD_COMPILER_VERSION=$(shell $(CC) -v 2>&1 | grep ' version ' | \
	sed 's/[[:space:]]*$$//')
endif

# Setup list of objects.mk files
ifdef PLATFORM
platform-object-mks=$(shell if [ -d $(platform_src_dir)/ ]; then find $(platform_src_dir) -iname "objects.mk" | sort -r; fi)
endif
libsbi-object-mks=$(shell if [ -d $(libsbi_dir) ]; then find $(libsbi_dir) -iname "objects.mk" | sort -r; fi)
libsbiutils-object-mks=$(shell if [ -d $(libsbiutils_dir) ]; then find $(libsbiutils_dir) -iname "objects.mk" | sort -r; fi)
firmware-object-mks=$(shell if [ -d $(firmware_dir) ]; then find $(firmware_dir) -iname "objects.mk" | sort -r; fi)

# The "make all" rule should always be first rule
.PHONY: all
all:

# Include platform specific .config
ifdef PLATFORM
.PHONY: menuconfig
menuconfig: $(platform_src_dir)/Kconfig $(src_dir)/Kconfig
	$(CMD_PREFIX)mkdir -p $(KCONFIG_DIR)
	$(CMD_PREFIX)$(src_dir)/scripts/Kconfiglib/menuconfig.py $(src_dir)/Kconfig
	$(CMD_PREFIX)$(src_dir)/scripts/Kconfiglib/genconfig.py --header-path $(KCONFIG_AUTOHEADER) --sync-deps $(KCONFIG_DIR) --file-list $(KCONFIG_AUTOLIST) $(src_dir)/Kconfig

.PHONY: savedefconfig
savedefconfig: $(platform_src_dir)/Kconfig $(src_dir)/Kconfig
	$(CMD_PREFIX)mkdir -p $(KCONFIG_DIR)
	$(CMD_PREFIX)$(src_dir)/scripts/Kconfiglib/savedefconfig.py --kconfig $(src_dir)/Kconfig --out $(KCONFIG_DIR)/defconfig

$(KCONFIG_CONFIG): $(platform_src_dir)/configs/$(PLATFORM_DEFCONFIG) $(platform_src_dir)/Kconfig $(src_dir)/Kconfig
	$(CMD_PREFIX)mkdir -p $(KCONFIG_DIR)
	$(CMD_PREFIX)$(src_dir)/scripts/Kconfiglib/defconfig.py --kconfig $(src_dir)/Kconfig $(platform_src_dir)/configs/$(PLATFORM_DEFCONFIG)
	$(CMD_PREFIX)$(src_dir)/scripts/Kconfiglib/genconfig.py --header-path $(KCONFIG_AUTOHEADER) --sync-deps $(KCONFIG_DIR) --file-list $(KCONFIG_AUTOLIST) $(src_dir)/Kconfig

$(KCONFIG_AUTOCMD): $(KCONFIG_CONFIG)
	$(CMD_PREFIX)mkdir -p $(KCONFIG_DIR)
	$(CMD_PREFIX)printf "%s: " $(KCONFIG_CONFIG) > $(KCONFIG_AUTOCMD)
	$(CMD_PREFIX)cat $(KCONFIG_AUTOLIST) | tr '\n' ' ' >> $(KCONFIG_AUTOCMD)

include $(KCONFIG_CONFIG)
include $(KCONFIG_AUTOCMD)
endif

# Include all objects.mk files
ifdef PLATFORM
include $(platform-object-mks)
endif
include $(libsbi-object-mks)
include $(libsbiutils-object-mks)
include $(firmware-object-mks)

# Setup list of objects
libsbi-objs-path-y=$(foreach obj,$(libsbi-objs-y),$(build_dir)/lib/sbi/$(obj))
ifdef PLATFORM
libsbiutils-objs-path-y=$(foreach obj,$(libsbiutils-objs-y),$(platform_build_dir)/lib/utils/$(obj))
platform-objs-path-y=$(foreach obj,$(platform-objs-y),$(platform_build_dir)/$(obj))
firmware-bins-path-y=$(foreach bin,$(firmware-bins-y),$(platform_build_dir)/firmware/$(bin))
endif
firmware-elfs-path-y=$(firmware-bins-path-y:.bin=.elf)
firmware-objs-path-y=$(firmware-bins-path-y:.bin=.o)

# Setup list of deps files for objects
deps-y=$(platform-objs-path-y:.o=.dep)
deps-y+=$(libsbi-objs-path-y:.o=.dep)
deps-y+=$(libsbiutils-objs-path-y:.o=.dep)
deps-y+=$(firmware-objs-path-y:.o=.dep)

# Setup platform ABI, ISA and Code Model
ifndef PLATFORM_RISCV_ABI
  ifneq ($(PLATFORM_RISCV_TOOLCHAIN_DEFAULT), 1)
    ifeq ($(PLATFORM_RISCV_XLEN), 32)
      PLATFORM_RISCV_ABI = ilp$(PLATFORM_RISCV_XLEN)
    else
      PLATFORM_RISCV_ABI = lp$(PLATFORM_RISCV_XLEN)
    endif
  else
    PLATFORM_RISCV_ABI = $(OPENSBI_CC_ABI)
  endif
endif
ifndef PLATFORM_RISCV_ISA
  ifneq ($(PLATFORM_RISCV_TOOLCHAIN_DEFAULT), 1)
    ifeq ($(CC_SUPPORT_ZICSR_ZIFENCEI), y)
      PLATFORM_RISCV_ISA = rv$(PLATFORM_RISCV_XLEN)imafdc_zicsr_zifencei
    else
      PLATFORM_RISCV_ISA = rv$(PLATFORM_RISCV_XLEN)imafdc
    endif
  else
    PLATFORM_RISCV_ISA = $(OPENSBI_CC_ISA)
  endif
endif
ifndef PLATFORM_RISCV_CODE_MODEL
  PLATFORM_RISCV_CODE_MODEL = medany
endif

# Setup install directories
ifdef INSTALL_INCLUDE_PATH
	install_include_path=$(INSTALL_INCLUDE_PATH)
else
	install_include_path=include
endif
ifdef INSTALL_LIB_PATH
	install_lib_path=$(INSTALL_LIB_PATH)
else
	ifneq ($(origin INSTALL_LIB_SUBDIR), undefined)
		install_lib_subdir=$(INSTALL_LIB_SUBDIR)
	else
		install_lib_subdir=$(PLATFORM_RISCV_ABI)
	endif
	install_lib_path=lib$(subst 32,,$(PLATFORM_RISCV_XLEN))/$(install_lib_subdir)
endif
ifdef INSTALL_FIRMWARE_PATH
	install_firmware_path=$(INSTALL_FIRMWARE_PATH)
else
	install_firmware_path=share/opensbi/$(PLATFORM_RISCV_ABI)
endif
ifdef INSTALL_DOCS_PATH
	install_docs_path=$(INSTALL_DOCS_PATH)
else
	install_docs_path=share/opensbi/docs
endif

# Setup compilation commands flags
ifeq ($(CC_IS_CLANG),y)
GENFLAGS	+=	$(CLANG_TARGET)
GENFLAGS	+=	-Wno-unused-command-line-argument
endif
GENFLAGS	+=	-I$(platform_src_dir)/include
GENFLAGS	+=	-I$(include_dir)
ifneq ($(OPENSBI_VERSION_GIT),)
GENFLAGS	+=	-DOPENSBI_VERSION_GIT="\"$(OPENSBI_VERSION_GIT)\""
endif
ifeq ($(BUILD_INFO),y)
GENFLAGS	+=	-DOPENSBI_BUILD_TIME_STAMP="\"$(OPENSBI_BUILD_TIME_STAMP)\""
GENFLAGS	+=	-DOPENSBI_BUILD_COMPILER_VERSION="\"$(OPENSBI_BUILD_COMPILER_VERSION)\""
endif
ifdef PLATFORM
GENFLAGS	+=	-include $(KCONFIG_AUTOHEADER)
endif
GENFLAGS	+=	$(libsbiutils-genflags-y)
GENFLAGS	+=	$(platform-genflags-y)
GENFLAGS	+=	$(firmware-genflags-y)

CFLAGS		=	-g -Wall -Werror -ffreestanding -nostdlib -fno-stack-protector -fno-strict-aliasing -O2
CFLAGS		+=	-fno-omit-frame-pointer -fno-optimize-sibling-calls -mstrict-align
# enable -m(no-)save-restore option by CC_SUPPORT_SAVE_RESTORE
ifeq ($(CC_SUPPORT_SAVE_RESTORE),y)
CFLAGS		+=	-mno-save-restore
endif
CFLAGS		+=	-mabi=$(PLATFORM_RISCV_ABI) -march=$(PLATFORM_RISCV_ISA)
CFLAGS		+=	-mcmodel=$(PLATFORM_RISCV_CODE_MODEL)
CFLAGS		+=	$(RELAX_FLAG)
CFLAGS		+=	$(GENFLAGS)
CFLAGS		+=	$(platform-cflags-y)
CFLAGS		+=	-fno-pie -no-pie
CFLAGS		+=	$(firmware-cflags-y)

CPPFLAGS	+=	$(GENFLAGS)
CPPFLAGS	+=	$(platform-cppflags-y)
CPPFLAGS	+=	$(firmware-cppflags-y)

ASFLAGS		=	-g -Wall -nostdlib
ASFLAGS		+=	-fno-omit-frame-pointer -fno-optimize-sibling-calls -mstrict-align
# enable -m(no-)save-restore option by CC_SUPPORT_SAVE_RESTORE
ifeq ($(CC_SUPPORT_SAVE_RESTORE),y)
ASFLAGS		+=	-mno-save-restore
endif
ASFLAGS		+=	-mabi=$(PLATFORM_RISCV_ABI) -march=$(PLATFORM_RISCV_ISA)
ASFLAGS		+=	-mcmodel=$(PLATFORM_RISCV_CODE_MODEL)
ASFLAGS		+=	$(RELAX_FLAG)
ifneq ($(CC_IS_CLANG),y)
ifneq ($(RELAX_FLAG),)
ASFLAGS		+=	-Wa,$(RELAX_FLAG)
endif
endif
ASFLAGS		+=	$(GENFLAGS)
ASFLAGS		+=	$(platform-asflags-y)
ASFLAGS		+=	$(firmware-asflags-y)

ARFLAGS		=	rcs

ELFFLAGS	+=	$(USE_LD_FLAG)
ELFFLAGS	+=	-Wl,--build-id=none -Wl,-N
ELFFLAGS	+=	$(platform-ldflags-y)
ELFFLAGS	+=	$(firmware-ldflags-y)

MERGEFLAGS	+=	-r
ifeq ($(LD_IS_LLD),y)
MERGEFLAGS	+=	-b elf
else
MERGEFLAGS	+=	-b elf$(PLATFORM_RISCV_XLEN)-littleriscv
endif
MERGEFLAGS	+=	-m elf$(PLATFORM_RISCV_XLEN)lriscv

DTSCPPFLAGS	=	$(CPPFLAGS) -nostdinc -nostdlib -fno-builtin -D__DTS__ -x assembler-with-cpp

# Setup functions for compilation
define dynamic_flags
-I$(shell dirname $(2)) -D__OBJNAME__=$(subst -,_,$(shell basename $(1) .o))
endef
merge_objs = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " MERGE     $(subst $(build_dir)/,,$(1))"; \
	     $(LD) $(MERGEFLAGS) $(2) -o $(1)
merge_deps = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " MERGE-DEP $(subst $(build_dir)/,,$(1))"; \
	     cat $(2) > $(1)
copy_file =  $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " COPY      $(subst $(build_dir)/,,$(1))"; \
	     cp -f $(2) $(1)
inst_file =  $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " INSTALL   $(subst $(install_root_dir)/,,$(1))"; \
	     cp -f $(2) $(1)
inst_file_list = $(CMD_PREFIX)if [ ! -z "$(4)" ]; then \
	     mkdir -p $(1)/$(3); \
	     for file in $(4) ; do \
	     rel_file=`echo $$file | sed -e 's@$(2)/$(subst $(install_firmware_path),platform,$(3))@@'`; \
	     dest_file=$(1)"/"$(3)"/"`echo $$rel_file`; \
	     dest_dir=`dirname $$dest_file`; \
	     echo " INSTALL   "$(3)"/"`echo $$rel_file`; \
	     mkdir -p $$dest_dir; \
	     cp -f $$file $$dest_file; \
	     done \
	     fi
inst_header_dir =  $(CMD_PREFIX)mkdir -p $(1); \
	     echo " INSTALL   $(subst $(install_root_dir)/,,$(1))"; \
	     cp -rf $(2) $(1)
compile_cpp = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " CPP       $(subst $(build_dir)/,,$(1))"; \
	     $(CPP) $(CPPFLAGS) -x c $(2) | grep -v "\#" > $(1)
compile_cc_dep = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " CC-DEP    $(subst $(build_dir)/,,$(1))"; \
	     printf %s `dirname $(1)`/  > $(1) && \
	     $(CC) $(CFLAGS) $(call dynamic_flags,$(1),$(2))   \
	       -MM $(2) >> $(1) || rm -f $(1)
compile_cc = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " CC        $(subst $(build_dir)/,,$(1))"; \
	     $(CC) $(CFLAGS) $(call dynamic_flags,$(1),$(2)) -c $(2) -o $(1)
compile_as_dep = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " AS-DEP    $(subst $(build_dir)/,,$(1))"; \
	     printf %s `dirname $(1)`/ > $(1) && \
	     $(AS) $(ASFLAGS) $(call dynamic_flags,$(1),$(2)) \
	       -MM $(2) >> $(1) || rm -f $(1)
compile_as = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " AS        $(subst $(build_dir)/,,$(1))"; \
	     $(AS) $(ASFLAGS) $(call dynamic_flags,$(1),$(2)) -c $(2) -o $(1)
compile_elf = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " ELF       $(subst $(build_dir)/,,$(1))"; \
	     $(CC) $(CFLAGS) $(3) $(ELFFLAGS) -Wl,-T$(2) -o $(1)
compile_ar = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " AR        $(subst $(build_dir)/,,$(1))"; \
	     $(AR) $(ARFLAGS) $(1) $(2)
compile_objcopy = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " OBJCOPY   $(subst $(build_dir)/,,$(1))"; \
	     $(OBJCOPY) -S -O binary $(2) $(1)
compile_dts = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " DTC       $(subst $(build_dir)/,,$(1))"; \
	     $(CPP) $(DTSCPPFLAGS) $(2) | $(DTC) -O dtb -i `dirname $(2)` -o $(1)
compile_d2c = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " D2C       $(subst $(build_dir)/,,$(1))"; \
	     $(if $($(2)-varalign-$(3)),$(eval D2C_ALIGN_BYTES := $($(2)-varalign-$(3))),$(eval D2C_ALIGN_BYTES := $(4))) \
	     $(if $($(2)-varprefix-$(3)),$(eval D2C_NAME_PREFIX := $($(2)-varprefix-$(3))),$(eval D2C_NAME_PREFIX := $(5))) \
	     $(if $($(2)-padding-$(3)),$(eval D2C_PADDING_BYTES := $($(2)-padding-$(3))),$(eval D2C_PADDING_BYTES := 0)) \
	     $(src_dir)/scripts/d2c.sh -i $(6) -a $(D2C_ALIGN_BYTES) -p $(D2C_NAME_PREFIX) -t $(D2C_PADDING_BYTES) > $(1)
compile_carray = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " CARRAY    $(subst $(build_dir)/,,$(1))"; \
	     $(eval CARRAY_VAR_LIST := $(carray-$(subst .c,,$(shell basename $(1)))-y)) \
	     $(src_dir)/scripts/carray.sh -i $(2) -l "$(CARRAY_VAR_LIST)" > $(1)
compile_gen_dep = $(CMD_PREFIX)mkdir -p `dirname $(1)`; \
	     echo " GEN-DEP   $(subst $(build_dir)/,,$(1))"; \
	     echo "$(1:.dep=$(2)): $(3)" >> $(1)

targets-y  = $(build_dir)/lib/libsbi.a
ifdef PLATFORM
targets-y += $(platform_build_dir)/lib/libplatsbi.a
endif
targets-y += $(firmware-bins-path-y)

# The default "make all" rule
.PHONY: all
all: $(targets-y)

# Preserve all intermediate files
.SECONDARY:

# Rules for lib/sbi sources
$(build_dir)/lib/libsbi.a: $(libsbi-objs-path-y)
	$(call compile_ar,$@,$^)

$(platform_build_dir)/lib/libplatsbi.a: $(libsbi-objs-path-y) $(libsbiutils-objs-path-y) $(platform-objs-path-y)
	$(call compile_ar,$@,$^)

$(build_dir)/%.dep: $(src_dir)/%.carray $(KCONFIG_CONFIG)
	$(call compile_gen_dep,$@,.c,$< $(KCONFIG_CONFIG))
	$(call compile_gen_dep,$@,.o,$(@:.dep=.c))

$(build_dir)/%.c: $(src_dir)/%.carray
	$(call compile_carray,$@,$<)

$(build_dir)/%.dep: $(src_dir)/%.c $(KCONFIG_CONFIG)
	$(call compile_cc_dep,$@,$<)

$(build_dir)/%.o: $(src_dir)/%.c
	$(call compile_cc,$@,$<)

$(build_dir)/%.o: $(build_dir)/%.c
	$(call compile_cc,$@,$<)

ifeq ($(BUILD_INFO),y)
$(build_dir)/lib/sbi/sbi_init.o: $(libsbi_dir)/sbi_init.c FORCE
	$(call compile_cc,$@,$<)
endif

$(build_dir)/%.dep: $(src_dir)/%.S $(KCONFIG_CONFIG)
	$(call compile_as_dep,$@,$<)

$(build_dir)/%.o: $(src_dir)/%.S
	$(call compile_as,$@,$<)

# Rules for platform sources
$(platform_build_dir)/%.dep: $(platform_src_dir)/%.carray $(KCONFIG_CONFIG)
	$(call compile_gen_dep,$@,.c,$< $(KCONFIG_CONFIG))
	$(call compile_gen_dep,$@,.o,$(@:.dep=.c))

$(platform_build_dir)/%.c: $(platform_src_dir)/%.carray
	$(call compile_carray,$@,$<)

$(platform_build_dir)/%.dep: $(platform_src_dir)/%.c $(KCONFIG_CONFIG)
	$(call compile_cc_dep,$@,$<)

$(platform_build_dir)/%.o: $(platform_src_dir)/%.c $(KCONFIG_CONFIG)
	$(call compile_cc,$@,$<)

$(platform_build_dir)/%.dep: $(platform_src_dir)/%.S
	$(call compile_as_dep,$@,$<)

$(platform_build_dir)/%.o: $(platform_src_dir)/%.S
	$(call compile_as,$@,$<)

$(platform_build_dir)/%.dep: $(platform_src_dir)/%.dts $(KCONFIG_CONFIG)
	$(call compile_gen_dep,$@,.dtb,$< $(KCONFIG_CONFIG))
	$(call compile_gen_dep,$@,.c,$(@:.dep=.dtb))
	$(call compile_gen_dep,$@,.o,$(@:.dep=.c))

$(platform_build_dir)/%.c: $(platform_build_dir)/%.dtb
	$(call compile_d2c,$@,platform,$(subst .dtb,.o,$(subst /,-,$(subst $(platform_build_dir)/,,$<))),16,dt,$<)

$(platform_build_dir)/%.dtb: $(platform_src_dir)/%.dts
	$(call compile_dts,$@,$<)

# Rules for lib/utils and firmware sources
$(platform_build_dir)/%.bin: $(platform_build_dir)/%.elf
	$(call compile_objcopy,$@,$<)

$(platform_build_dir)/%.elf: $(platform_build_dir)/%.o $(platform_build_dir)/%.elf.ld $(platform_build_dir)/lib/libplatsbi.a
	$(call compile_elf,$@,$@.ld,$< $(platform_build_dir)/lib/libplatsbi.a)

$(platform_build_dir)/%.ld: $(src_dir)/%.ldS
	$(call compile_cpp,$@,$<)

$(platform_build_dir)/%.dep: $(src_dir)/%.carray $(KCONFIG_CONFIG)
	$(call compile_gen_dep,$@,.c,$< $(KCONFIG_CONFIG))
	$(call compile_gen_dep,$@,.o,$(@:.dep=.c))

$(platform_build_dir)/%.c: $(src_dir)/%.carray
	$(call compile_carray,$@,$<)

$(platform_build_dir)/%.dep: $(src_dir)/%.c $(KCONFIG_CONFIG)
	$(call compile_cc_dep,$@,$<)

$(platform_build_dir)/%.o: $(src_dir)/%.c
	$(call compile_cc,$@,$<)

$(platform_build_dir)/%.dep: $(src_dir)/%.S $(KCONFIG_CONFIG)
	$(call compile_as_dep,$@,$<)

$(platform_build_dir)/%.o: $(src_dir)/%.S
	$(call compile_as,$@,$<)

# Rule for "make docs"
$(build_dir)/docs/latex/refman.pdf: $(build_dir)/docs/latex/refman.tex
	$(CMD_PREFIX)mkdir -p $(build_dir)/docs
	$(CMD_PREFIX)$(MAKE) -C $(build_dir)/docs/latex
$(build_dir)/docs/latex/refman.tex: $(build_dir)/docs/doxygen.cfg
	$(CMD_PREFIX)mkdir -p $(build_dir)/docs
	$(CMD_PREFIX)doxygen $(build_dir)/docs/doxygen.cfg
$(build_dir)/docs/doxygen.cfg: $(src_dir)/docs/doxygen.cfg
	$(CMD_PREFIX)mkdir -p $(build_dir)/docs
	$(CMD_PREFIX)cat docs/doxygen.cfg | sed -e "s#@@SRC_DIR@@#$(src_dir)#" -e "s#@@BUILD_DIR@@#$(build_dir)#" -e "s#@@OPENSBI_MAJOR@@#$(OPENSBI_VERSION_MAJOR)#" -e "s#@@OPENSBI_MINOR@@#$(OPENSBI_VERSION_MINOR)#" > $(build_dir)/docs/doxygen.cfg
.PHONY: docs
docs: $(build_dir)/docs/latex/refman.pdf

# Dependency files should only be included after default Makefile rules
# They should not be included for any "xxxconfig" or "xxxclean" rule
all-deps-1 = $(if $(findstring config,$(MAKECMDGOALS)),,$(deps-y))
all-deps-2 = $(if $(findstring clean,$(MAKECMDGOALS)),,$(all-deps-1))
-include $(all-deps-2)

# Include external dependency of firmwares after default Makefile rules
include $(src_dir)/firmware/external_deps.mk

# Convenient "make run" command for emulated platforms
.PHONY: run
run: all
ifneq ($(platform-runcmd),)
	$(platform-runcmd) $(RUN_ARGS)
else
ifdef PLATFORM
	@echo "Platform $(PLATFORM) doesn't specify a run command"
	@false
else
	@echo Run command only available when targeting a platform
	@false
endif
endif

install_targets-y  = install_libsbi
ifdef PLATFORM
install_targets-y += install_libplatsbi
install_targets-y += install_firmwares
endif

# Rule for "make install"
.PHONY: install
install: $(install_targets-y)

.PHONY: install_libsbi
install_libsbi: $(build_dir)/lib/libsbi.a
	$(call inst_header_dir,$(install_root_dir)/$(install_include_path),$(include_dir)/sbi)
	$(call inst_file,$(install_root_dir)/$(install_lib_path)/libsbi.a,$(build_dir)/lib/libsbi.a)

.PHONY: install_libplatsbi
install_libplatsbi: $(platform_build_dir)/lib/libplatsbi.a $(build_dir)/lib/libsbi.a
	$(call inst_file,$(install_root_dir)/$(install_lib_path)/opensbi/$(platform_subdir)/lib/libplatsbi.a,$(platform_build_dir)/lib/libplatsbi.a)

.PHONY: install_firmwares
install_firmwares: $(platform_build_dir)/lib/libplatsbi.a $(build_dir)/lib/libsbi.a $(firmware-bins-path-y)
	$(call inst_file_list,$(install_root_dir),$(build_dir),$(install_firmware_path)/$(platform_subdir)/firmware,$(firmware-elfs-path-y))
	$(call inst_file_list,$(install_root_dir),$(build_dir),$(install_firmware_path)/$(platform_subdir)/firmware,$(firmware-bins-path-y))

.PHONY: install_docs
install_docs: $(build_dir)/docs/latex/refman.pdf
	$(call inst_file,$(install_root_dir)/$(install_docs_path)/refman.pdf,$(build_dir)/docs/latex/refman.pdf)

.PHONY: cscope
cscope:
	$(CMD_PREFIX)find \
		"$(src_dir)/firmware" \
		"$(src_dir)/include" \
		"$(src_dir)/lib" \
		"$(platform_src_dir)" \
	-name "*.[chS]" -print > cscope.files
	$(CMD_PREFIX)echo "$(KCONFIG_AUTOHEADER)" >> cscope.files
	$(CMD_PREFIX)cscope -bkq -i cscope.files -f cscope.out

# Rule for "make clean"
.PHONY: clean
clean:
	$(CMD_PREFIX)mkdir -p $(build_dir)
	$(if $(V), @echo " RM        $(build_dir)/*.o")
	$(CMD_PREFIX)find $(build_dir) -type f -name "*.o" -exec rm -rf {} +
	$(if $(V), @echo " RM        $(build_dir)/*.a")
	$(CMD_PREFIX)find $(build_dir) -type f -name "*.a" -exec rm -rf {} +
	$(if $(V), @echo " RM        $(build_dir)/*.elf")
	$(CMD_PREFIX)find $(build_dir) -type f -name "*.elf" -exec rm -rf {} +
	$(if $(V), @echo " RM        $(build_dir)/*.bin")
	$(CMD_PREFIX)find $(build_dir) -type f -name "*.bin" -exec rm -rf {} +
	$(if $(V), @echo " RM        $(build_dir)/*.dtb")
	$(CMD_PREFIX)find $(build_dir) -type f -name "*.dtb" -exec rm -rf {} +

# Rule for "make distclean"
.PHONY: distclean
distclean: clean
	$(CMD_PREFIX)mkdir -p $(build_dir)
	$(if $(V), @echo " RM        $(build_dir)/*.dep")
	$(CMD_PREFIX)find $(build_dir) -type f -name "*.dep" -exec rm -rf {} +
ifeq ($(build_dir),$(CURDIR)/build)
	$(if $(V), @echo " RM        $(build_dir)")
	$(CMD_PREFIX)rm -rf $(build_dir)
endif
ifeq ($(install_root_dir),$(install_root_dir_default)/usr)
	$(if $(V), @echo " RM        $(install_root_dir_default)")
	$(CMD_PREFIX)rm -rf $(install_root_dir_default)
endif
	$(if $(V), @echo " RM        $(src_dir)/cscope*")
	$(CMD_PREFIX)rm -f $(src_dir)/cscope*

.PHONY: FORCE
FORCE:
