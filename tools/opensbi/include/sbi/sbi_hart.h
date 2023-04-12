/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#ifndef __SBI_HART_H__
#define __SBI_HART_H__

#include <sbi/sbi_types.h>

/** Possible privileged specification versions of a hart */
enum sbi_hart_priv_versions {
	/** Unknown privileged specification */
	SBI_HART_PRIV_VER_UNKNOWN = 0,
	/** Privileged specification v1.10 */
	SBI_HART_PRIV_VER_1_10 = 1,
	/** Privileged specification v1.11 */
	SBI_HART_PRIV_VER_1_11 = 2,
	/** Privileged specification v1.12 */
	SBI_HART_PRIV_VER_1_12 = 3,
};

/** Possible ISA extensions of a hart */
enum sbi_hart_extensions {
	/** Hart has Sscofpmt extension */
	SBI_HART_EXT_SSCOFPMF = 0,
	/** HART has HW time CSR (extension name not available) */
	SBI_HART_EXT_TIME,
	/** HART has AIA M-mode CSRs */
	SBI_HART_EXT_SMAIA,
	/** HART has Smstateen CSR **/
	SBI_HART_EXT_SMSTATEEN,
	/** HART has Sstc extension */
	SBI_HART_EXT_SSTC,

	/** Maximum index of Hart extension */
	SBI_HART_EXT_MAX,
};

struct sbi_hart_features {
	bool detected;
	int priv_version;
	unsigned long extensions;
	unsigned int pmp_count;
	unsigned int pmp_addr_bits;
	unsigned long pmp_gran;
	unsigned int mhpm_count;
	unsigned int mhpm_bits;
};

struct sbi_scratch;

int sbi_hart_reinit(struct sbi_scratch *scratch);
int sbi_hart_init(struct sbi_scratch *scratch, bool cold_boot);

extern void (*sbi_hart_expected_trap)(void);
static inline ulong sbi_hart_expected_trap_addr(void)
{
	return (ulong)sbi_hart_expected_trap;
}

unsigned int sbi_hart_mhpm_count(struct sbi_scratch *scratch);
void sbi_hart_delegation_dump(struct sbi_scratch *scratch,
			      const char *prefix, const char *suffix);
unsigned int sbi_hart_pmp_count(struct sbi_scratch *scratch);
unsigned long sbi_hart_pmp_granularity(struct sbi_scratch *scratch);
unsigned int sbi_hart_pmp_addrbits(struct sbi_scratch *scratch);
unsigned int sbi_hart_mhpm_bits(struct sbi_scratch *scratch);
int sbi_hart_pmp_configure(struct sbi_scratch *scratch);
int sbi_hart_priv_version(struct sbi_scratch *scratch);
void sbi_hart_get_priv_version_str(struct sbi_scratch *scratch,
				   char *version_str, int nvstr);
void sbi_hart_update_extension(struct sbi_scratch *scratch,
			       enum sbi_hart_extensions ext,
			       bool enable);
bool sbi_hart_has_extension(struct sbi_scratch *scratch,
			    enum sbi_hart_extensions ext);
void sbi_hart_get_extensions_str(struct sbi_scratch *scratch,
				 char *extension_str, int nestr);

void __attribute__((noreturn)) sbi_hart_hang(void);

void __attribute__((noreturn))
sbi_hart_switch_mode(unsigned long arg0, unsigned long arg1,
		     unsigned long next_addr, unsigned long next_mode,
		     bool next_virt);

#endif
