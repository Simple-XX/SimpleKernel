/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#ifndef __FW_DYNAMIC_H__
#define __FW_DYNAMIC_H__

#include <sbi/riscv_asm.h>

/* clang-format off */

/** Offset of magic member in fw_dynamic_info */
#define FW_DYNAMIC_INFO_MAGIC_OFFSET		(0 * __SIZEOF_LONG__)
/** Offset of version member in fw_dynamic_info */
#define FW_DYNAMIC_INFO_VERSION_OFFSET		(1 * __SIZEOF_LONG__)
/** Offset of next_addr member in fw_dynamic_info (version >= 1) */
#define FW_DYNAMIC_INFO_NEXT_ADDR_OFFSET	(2 * __SIZEOF_LONG__)
/** Offset of next_mode member in fw_dynamic_info  (version >= 1) */
#define FW_DYNAMIC_INFO_NEXT_MODE_OFFSET	(3 * __SIZEOF_LONG__)
/** Offset of options member in fw_dynamic_info  (version >= 1) */
#define FW_DYNAMIC_INFO_OPTIONS_OFFSET		(4 * __SIZEOF_LONG__)
/** Offset of boot_hart member in fw_dynamic_info  (version >= 2) */
#define FW_DYNAMIC_INFO_BOOT_HART_OFFSET	(5 * __SIZEOF_LONG__)

/** Expected value of info magic ('OSBI' ascii string in hex) */
#define FW_DYNAMIC_INFO_MAGIC_VALUE		0x4942534f

/** Maximum supported info version */
#define FW_DYNAMIC_INFO_VERSION_2		0x2
#define FW_DYNAMIC_INFO_VERSION_MAX		FW_DYNAMIC_INFO_VERSION_2

/** Possible next mode values */
#define FW_DYNAMIC_INFO_NEXT_MODE_U		0x0
#define FW_DYNAMIC_INFO_NEXT_MODE_S		0x1
#define FW_DYNAMIC_INFO_NEXT_MODE_M		0x3

/* clang-format on */

#ifndef __ASSEMBLER__

#include <sbi/sbi_types.h>

/** Representation dynamic info passed by previous booting stage */
struct fw_dynamic_info {
	/** Info magic */
	unsigned long magic;
	/** Info version */
	unsigned long version;
	/** Next booting stage address */
	unsigned long next_addr;
	/** Next booting stage mode */
	unsigned long next_mode;
	/** Options for OpenSBI library */
	unsigned long options;
	/**
	 * Preferred boot HART id
	 *
	 * It is possible that the previous booting stage uses same link
	 * address as the FW_DYNAMIC firmware. In this case, the relocation
	 * lottery mechanism can potentially overwrite the previous booting
	 * stage while other HARTs are still running in the previous booting
	 * stage leading to boot-time crash. To avoid this boot-time crash,
	 * the previous booting stage can specify last HART that will jump
	 * to the FW_DYNAMIC firmware as the preferred boot HART.
	 *
	 * To avoid specifying a preferred boot HART, the previous booting
	 * stage can set it to -1UL which will force the FW_DYNAMIC firmware
	 * to use the relocation lottery mechanism.
	 */
	unsigned long boot_hart;
} __packed;

/**
 * Prevent modification of struct fw_dynamic_info from affecting
 * FW_DYNAMIC_INFO_xxx_OFFSET
 */
_Static_assert(
	offsetof(struct fw_dynamic_info, magic)
		== FW_DYNAMIC_INFO_MAGIC_OFFSET,
	"struct fw_dynamic_info definition has changed, please redefine "
	"FW_DYNAMIC_INFO_MAGIC_OFFSET");
_Static_assert(
	offsetof(struct fw_dynamic_info, version)
		== FW_DYNAMIC_INFO_VERSION_OFFSET,
	"struct fw_dynamic_info definition has changed, please redefine "
	"FW_DYNAMIC_INFO_VERSION_OFFSET");
_Static_assert(
	offsetof(struct fw_dynamic_info, next_addr)
		== FW_DYNAMIC_INFO_NEXT_ADDR_OFFSET,
	"struct fw_dynamic_info definition has changed, please redefine "
	"FW_DYNAMIC_INFO_NEXT_ADDR_OFFSET");
_Static_assert(
	offsetof(struct fw_dynamic_info, next_mode)
		== FW_DYNAMIC_INFO_NEXT_MODE_OFFSET,
	"struct fw_dynamic_info definition has changed, please redefine "
	"FW_DYNAMIC_INFO_NEXT_MODE_OFFSET");
_Static_assert(
	offsetof(struct fw_dynamic_info, options)
		== FW_DYNAMIC_INFO_OPTIONS_OFFSET,
	"struct fw_dynamic_info definition has changed, please redefine "
	"FW_DYNAMIC_INFO_OPTIONS_OFFSET");
_Static_assert(
	offsetof(struct fw_dynamic_info, boot_hart)
		== FW_DYNAMIC_INFO_BOOT_HART_OFFSET,
	"struct fw_dynamic_info definition has changed, please redefine "
	"FW_DYNAMIC_INFO_BOOT_HART_OFFSET");

#endif

#endif
