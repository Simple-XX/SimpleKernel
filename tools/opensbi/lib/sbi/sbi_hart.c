/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#include <sbi/riscv_asm.h>
#include <sbi/riscv_barrier.h>
#include <sbi/riscv_encoding.h>
#include <sbi/riscv_fp.h>
#include <sbi/sbi_bitops.h>
#include <sbi/sbi_console.h>
#include <sbi/sbi_domain.h>
#include <sbi/sbi_csr_detect.h>
#include <sbi/sbi_error.h>
#include <sbi/sbi_hart.h>
#include <sbi/sbi_math.h>
#include <sbi/sbi_platform.h>
#include <sbi/sbi_pmu.h>
#include <sbi/sbi_string.h>
#include <sbi/sbi_trap.h>
#include <sbi/sbi_hfence.h>

extern void __sbi_expected_trap(void);
extern void __sbi_expected_trap_hext(void);

void (*sbi_hart_expected_trap)(void) = &__sbi_expected_trap;

static unsigned long hart_features_offset;

static void mstatus_init(struct sbi_scratch *scratch)
{
	unsigned long menvcfg_val, mstatus_val = 0;
	int cidx;
	unsigned int num_mhpm = sbi_hart_mhpm_count(scratch);
	uint64_t mhpmevent_init_val = 0;
	uint64_t mstateen_val;

	/* Enable FPU */
	if (misa_extension('D') || misa_extension('F'))
		mstatus_val |=  MSTATUS_FS;

	/* Enable Vector context */
	if (misa_extension('V'))
		mstatus_val |=  MSTATUS_VS;

	csr_write(CSR_MSTATUS, mstatus_val);

	/* Disable user mode usage of all perf counters except default ones (CY, TM, IR) */
	if (misa_extension('S') &&
	    sbi_hart_priv_version(scratch) >= SBI_HART_PRIV_VER_1_10)
		csr_write(CSR_SCOUNTEREN, 7);

	/**
	 * OpenSBI doesn't use any PMU counters in M-mode.
	 * Supervisor mode usage for all counters are enabled by default
	 * But counters will not run until mcountinhibit is set.
	 */
	if (sbi_hart_priv_version(scratch) >= SBI_HART_PRIV_VER_1_10)
		csr_write(CSR_MCOUNTEREN, -1);

	/* All programmable counters will start running at runtime after S-mode request */
	if (sbi_hart_priv_version(scratch) >= SBI_HART_PRIV_VER_1_11)
		csr_write(CSR_MCOUNTINHIBIT, 0xFFFFFFF8);

	/**
	 * The mhpmeventn[h] CSR should be initialized with interrupt disabled
	 * and inhibited running in M-mode during init.
	 * To keep it simple, only contiguous mhpmcounters are supported as a
	 * platform with discontiguous mhpmcounters may not make much sense.
	 */
	mhpmevent_init_val |= (MHPMEVENT_OF | MHPMEVENT_MINH);
	for (cidx = 0; cidx < num_mhpm; cidx++) {
#if __riscv_xlen == 32
		csr_write_num(CSR_MHPMEVENT3 + cidx, mhpmevent_init_val & 0xFFFFFFFF);
		if (sbi_hart_has_extension(scratch, SBI_HART_EXT_SSCOFPMF))
			csr_write_num(CSR_MHPMEVENT3H + cidx,
				      mhpmevent_init_val >> BITS_PER_LONG);
#else
		csr_write_num(CSR_MHPMEVENT3 + cidx, mhpmevent_init_val);
#endif
	}

	if (sbi_hart_has_extension(scratch, SBI_HART_EXT_SMSTATEEN)) {
		mstateen_val = csr_read(CSR_MSTATEEN0);
#if __riscv_xlen == 32
		mstateen_val |= ((uint64_t)csr_read(CSR_MSTATEEN0H)) << 32;
#endif
		mstateen_val |= SMSTATEEN_STATEN;
		mstateen_val |= SMSTATEEN0_HSENVCFG;

		if (sbi_hart_has_extension(scratch, SBI_HART_EXT_SMAIA))
			mstateen_val |= (SMSTATEEN0_AIA | SMSTATEEN0_SVSLCT |
					SMSTATEEN0_IMSIC);
		else
			mstateen_val &= ~(SMSTATEEN0_AIA | SMSTATEEN0_SVSLCT |
					SMSTATEEN0_IMSIC);
		csr_write(CSR_MSTATEEN0, mstateen_val);
#if __riscv_xlen == 32
		csr_write(CSR_MSTATEEN0H, mstateen_val >> 32);
#endif
	}

	if (sbi_hart_priv_version(scratch) >= SBI_HART_PRIV_VER_1_12) {
		menvcfg_val = csr_read(CSR_MENVCFG);

		/*
		 * Set menvcfg.CBZE == 1
		 *
		 * If Zicboz extension is not available then writes to
		 * menvcfg.CBZE will be ignored because it is a WARL field.
		 */
		menvcfg_val |= ENVCFG_CBZE;

		/*
		 * Set menvcfg.CBCFE == 1
		 *
		 * If Zicbom extension is not available then writes to
		 * menvcfg.CBCFE will be ignored because it is a WARL field.
		 */
		menvcfg_val |= ENVCFG_CBCFE;

		/*
		 * Set menvcfg.CBIE == 3
		 *
		 * If Zicbom extension is not available then writes to
		 * menvcfg.CBIE will be ignored because it is a WARL field.
		 */
		menvcfg_val |= ENVCFG_CBIE_INV << ENVCFG_CBIE_SHIFT;

		/*
		 * Set menvcfg.PBMTE == 1 for RV64 or RV128
		 *
		 * If Svpbmt extension is not available then menvcfg.PBMTE
		 * will be read-only zero.
		 */
#if __riscv_xlen > 32
		menvcfg_val |= ENVCFG_PBMTE;
#endif

		/*
		 * The spec doesn't explicitly describe the reset value of menvcfg.
		 * Enable access to stimecmp if sstc extension is present in the
		 * hardware.
		 */
		if (sbi_hart_has_extension(scratch, SBI_HART_EXT_SSTC)) {
#if __riscv_xlen == 32
			unsigned long menvcfgh_val;
			menvcfgh_val = csr_read(CSR_MENVCFGH);
			menvcfgh_val |= ENVCFGH_STCE;
			csr_write(CSR_MENVCFGH, menvcfgh_val);
#else
			menvcfg_val |= ENVCFG_STCE;
#endif
		}

		csr_write(CSR_MENVCFG, menvcfg_val);
	}

	/* Disable all interrupts */
	csr_write(CSR_MIE, 0);

	/* Disable S-mode paging */
	if (misa_extension('S'))
		csr_write(CSR_SATP, 0);
}

static int fp_init(struct sbi_scratch *scratch)
{
#ifdef __riscv_flen
	int i;
#endif

	if (!misa_extension('D') && !misa_extension('F'))
		return 0;

	if (!(csr_read(CSR_MSTATUS) & MSTATUS_FS))
		return SBI_EINVAL;

#ifdef __riscv_flen
	for (i = 0; i < 32; i++)
		init_fp_reg(i);
	csr_write(CSR_FCSR, 0);
#endif

	return 0;
}

static int delegate_traps(struct sbi_scratch *scratch)
{
	const struct sbi_platform *plat = sbi_platform_ptr(scratch);
	unsigned long interrupts, exceptions;

	if (!misa_extension('S'))
		/* No delegation possible as mideleg does not exist */
		return 0;

	/* Send M-mode interrupts and most exceptions to S-mode */
	interrupts = MIP_SSIP | MIP_STIP | MIP_SEIP;
	interrupts |= sbi_pmu_irq_bit();

	exceptions = (1U << CAUSE_MISALIGNED_FETCH) | (1U << CAUSE_BREAKPOINT) |
		     (1U << CAUSE_USER_ECALL);
	if (sbi_platform_has_mfaults_delegation(plat))
		exceptions |= (1U << CAUSE_FETCH_PAGE_FAULT) |
			      (1U << CAUSE_LOAD_PAGE_FAULT) |
			      (1U << CAUSE_STORE_PAGE_FAULT);

	/*
	 * If hypervisor extension available then we only handle hypervisor
	 * calls (i.e. ecalls from HS-mode) in M-mode.
	 *
	 * The HS-mode will additionally handle supervisor calls (i.e. ecalls
	 * from VS-mode), Guest page faults and Virtual interrupts.
	 */
	if (misa_extension('H')) {
		exceptions |= (1U << CAUSE_VIRTUAL_SUPERVISOR_ECALL);
		exceptions |= (1U << CAUSE_FETCH_GUEST_PAGE_FAULT);
		exceptions |= (1U << CAUSE_LOAD_GUEST_PAGE_FAULT);
		exceptions |= (1U << CAUSE_VIRTUAL_INST_FAULT);
		exceptions |= (1U << CAUSE_STORE_GUEST_PAGE_FAULT);
	}

	csr_write(CSR_MIDELEG, interrupts);
	csr_write(CSR_MEDELEG, exceptions);

	return 0;
}

void sbi_hart_delegation_dump(struct sbi_scratch *scratch,
			      const char *prefix, const char *suffix)
{
	if (!misa_extension('S'))
		/* No delegation possible as mideleg does not exist*/
		return;

	sbi_printf("%sMIDELEG%s: 0x%" PRILX "\n",
		   prefix, suffix, csr_read(CSR_MIDELEG));
	sbi_printf("%sMEDELEG%s: 0x%" PRILX "\n",
		   prefix, suffix, csr_read(CSR_MEDELEG));
}

unsigned int sbi_hart_mhpm_count(struct sbi_scratch *scratch)
{
	struct sbi_hart_features *hfeatures =
			sbi_scratch_offset_ptr(scratch, hart_features_offset);

	return hfeatures->mhpm_count;
}

unsigned int sbi_hart_pmp_count(struct sbi_scratch *scratch)
{
	struct sbi_hart_features *hfeatures =
			sbi_scratch_offset_ptr(scratch, hart_features_offset);

	return hfeatures->pmp_count;
}

unsigned long sbi_hart_pmp_granularity(struct sbi_scratch *scratch)
{
	struct sbi_hart_features *hfeatures =
			sbi_scratch_offset_ptr(scratch, hart_features_offset);

	return hfeatures->pmp_gran;
}

unsigned int sbi_hart_pmp_addrbits(struct sbi_scratch *scratch)
{
	struct sbi_hart_features *hfeatures =
			sbi_scratch_offset_ptr(scratch, hart_features_offset);

	return hfeatures->pmp_addr_bits;
}

unsigned int sbi_hart_mhpm_bits(struct sbi_scratch *scratch)
{
	struct sbi_hart_features *hfeatures =
			sbi_scratch_offset_ptr(scratch, hart_features_offset);

	return hfeatures->mhpm_bits;
}

int sbi_hart_pmp_configure(struct sbi_scratch *scratch)
{
	struct sbi_domain_memregion *reg;
	struct sbi_domain *dom = sbi_domain_thishart_ptr();
	unsigned int pmp_idx = 0, pmp_flags, pmp_bits, pmp_gran_log2;
	unsigned int pmp_count = sbi_hart_pmp_count(scratch);
	unsigned long pmp_addr = 0, pmp_addr_max = 0;

	if (!pmp_count)
		return 0;

	pmp_gran_log2 = log2roundup(sbi_hart_pmp_granularity(scratch));
	pmp_bits = sbi_hart_pmp_addrbits(scratch) - 1;
	pmp_addr_max = (1UL << pmp_bits) | ((1UL << pmp_bits) - 1);

	sbi_domain_for_each_memregion(dom, reg) {
		if (pmp_count <= pmp_idx)
			break;

		pmp_flags = 0;
		if (reg->flags & SBI_DOMAIN_MEMREGION_READABLE)
			pmp_flags |= PMP_R;
		if (reg->flags & SBI_DOMAIN_MEMREGION_WRITEABLE)
			pmp_flags |= PMP_W;
		if (reg->flags & SBI_DOMAIN_MEMREGION_EXECUTABLE)
			pmp_flags |= PMP_X;
		if (reg->flags & SBI_DOMAIN_MEMREGION_MMODE)
			pmp_flags |= PMP_L;

		pmp_addr =  reg->base >> PMP_SHIFT;
		if (pmp_gran_log2 <= reg->order && pmp_addr < pmp_addr_max)
			pmp_set(pmp_idx++, pmp_flags, reg->base, reg->order);
		else {
			sbi_printf("Can not configure pmp for domain %s", dom->name);
			sbi_printf(" because memory region address %lx or size %lx is not in range\n",
				    reg->base, reg->order);
		}
	}

	/*
	 * As per section 3.7.2 of privileged specification v1.12,
	 * virtual address translations can be speculatively performed
	 * (even before actual access). These, along with PMP traslations,
	 * can be cached. This can pose a problem with CPU hotplug
	 * and non-retentive suspend scenario because PMP states are
	 * not preserved.
	 * It is advisable to flush the caching structures under such
	 * conditions.
	 */
	if (misa_extension('S')) {
		__asm__ __volatile__("sfence.vma");

		/*
		 * If hypervisor mode is supported, flush caching
		 * structures in guest mode too.
		 */
		if (misa_extension('H'))
			__sbi_hfence_gvma_all();
	}

	return 0;
}

int sbi_hart_priv_version(struct sbi_scratch *scratch)
{
	struct sbi_hart_features *hfeatures =
			sbi_scratch_offset_ptr(scratch, hart_features_offset);

	return hfeatures->priv_version;
}

void sbi_hart_get_priv_version_str(struct sbi_scratch *scratch,
				   char *version_str, int nvstr)
{
	char *temp;
	struct sbi_hart_features *hfeatures =
			sbi_scratch_offset_ptr(scratch, hart_features_offset);

	switch (hfeatures->priv_version) {
	case SBI_HART_PRIV_VER_1_10:
		temp = "v1.10";
		break;
	case SBI_HART_PRIV_VER_1_11:
		temp = "v1.11";
		break;
	case SBI_HART_PRIV_VER_1_12:
		temp = "v1.12";
		break;
	default:
		temp = "unknown";
		break;
	}

	sbi_snprintf(version_str, nvstr, "%s", temp);
}

static inline void __sbi_hart_update_extension(
					struct sbi_hart_features *hfeatures,
					enum sbi_hart_extensions ext,
					bool enable)
{
	if (enable)
		hfeatures->extensions |= BIT(ext);
	else
		hfeatures->extensions &= ~BIT(ext);
}

/**
 * Enable/Disable a particular hart extension
 *
 * @param scratch pointer to the HART scratch space
 * @param ext the extension number to check
 * @param enable new state of hart extension
 */
void sbi_hart_update_extension(struct sbi_scratch *scratch,
			       enum sbi_hart_extensions ext,
			       bool enable)
{
	struct sbi_hart_features *hfeatures =
			sbi_scratch_offset_ptr(scratch, hart_features_offset);

	__sbi_hart_update_extension(hfeatures, ext, enable);
}

/**
 * Check whether a particular hart extension is available
 *
 * @param scratch pointer to the HART scratch space
 * @param ext the extension number to check
 * @returns true (available) or false (not available)
 */
bool sbi_hart_has_extension(struct sbi_scratch *scratch,
			    enum sbi_hart_extensions ext)
{
	struct sbi_hart_features *hfeatures =
			sbi_scratch_offset_ptr(scratch, hart_features_offset);

	if (hfeatures->extensions & BIT(ext))
		return true;
	else
		return false;
}

static inline char *sbi_hart_extension_id2string(int ext)
{
	char *estr = NULL;

	switch (ext) {
	case SBI_HART_EXT_SSCOFPMF:
		estr = "sscofpmf";
		break;
	case SBI_HART_EXT_TIME:
		estr = "time";
		break;
	case SBI_HART_EXT_SMAIA:
		estr = "smaia";
		break;
	case SBI_HART_EXT_SSTC:
		estr = "sstc";
		break;
	case SBI_HART_EXT_SMSTATEEN:
		estr = "smstateen";
		break;
	default:
		break;
	}

	return estr;
}

/**
 * Get the hart extensions in string format
 *
 * @param scratch pointer to the HART scratch space
 * @param extensions_str pointer to a char array where the extensions string
 *			 will be updated
 * @param nestr length of the features_str. The feature string will be
 *		truncated if nestr is not long enough.
 */
void sbi_hart_get_extensions_str(struct sbi_scratch *scratch,
				 char *extensions_str, int nestr)
{
	struct sbi_hart_features *hfeatures =
			sbi_scratch_offset_ptr(scratch, hart_features_offset);
	int offset = 0, ext = 0;
	char *temp;

	if (!extensions_str || nestr <= 0)
		return;
	sbi_memset(extensions_str, 0, nestr);

	if (!hfeatures->extensions)
		goto done;

	do {
		if (hfeatures->extensions & BIT(ext)) {
			temp = sbi_hart_extension_id2string(ext);
			if (temp) {
				sbi_snprintf(extensions_str + offset,
					     nestr - offset,
					     "%s,", temp);
				offset = offset + sbi_strlen(temp) + 1;
			}
		}

		ext++;
	} while (ext < SBI_HART_EXT_MAX);

done:
	if (offset)
		extensions_str[offset - 1] = '\0';
	else
		sbi_strncpy(extensions_str, "none", nestr);
}

static unsigned long hart_pmp_get_allowed_addr(void)
{
	unsigned long val = 0;
	struct sbi_trap_info trap = {0};

	csr_write_allowed(CSR_PMPCFG0, (ulong)&trap, 0);
	if (trap.cause)
		return 0;

	csr_write_allowed(CSR_PMPADDR0, (ulong)&trap, PMP_ADDR_MASK);
	if (!trap.cause) {
		val = csr_read_allowed(CSR_PMPADDR0, (ulong)&trap);
		if (trap.cause)
			val = 0;
	}

	return val;
}

static int hart_pmu_get_allowed_bits(void)
{
	unsigned long val = ~(0UL);
	struct sbi_trap_info trap = {0};
	int num_bits = 0;

	/**
	 * It is assumed that platforms will implement same number of bits for
	 * all the performance counters including mcycle/minstret.
	 */
	csr_write_allowed(CSR_MHPMCOUNTER3, (ulong)&trap, val);
	if (!trap.cause) {
		val = csr_read_allowed(CSR_MHPMCOUNTER3, (ulong)&trap);
		if (trap.cause)
			return 0;
	}
	num_bits = sbi_fls(val) + 1;
#if __riscv_xlen == 32
	csr_write_allowed(CSR_MHPMCOUNTER3H, (ulong)&trap, val);
	if (!trap.cause) {
		val = csr_read_allowed(CSR_MHPMCOUNTER3H, (ulong)&trap);
		if (trap.cause)
			return num_bits;
	}
	num_bits += sbi_fls(val) + 1;

#endif

	return num_bits;
}

static int hart_detect_features(struct sbi_scratch *scratch)
{
	struct sbi_trap_info trap = {0};
	struct sbi_hart_features *hfeatures =
		sbi_scratch_offset_ptr(scratch, hart_features_offset);
	unsigned long val, oldval;
	int rc;

	/* If hart features already detected then do nothing */
	if (hfeatures->detected)
		return 0;

	/* Clear hart features */
	hfeatures->extensions = 0;
	hfeatures->pmp_count = 0;
	hfeatures->mhpm_count = 0;

#define __check_csr(__csr, __rdonly, __wrval, __field, __skip)	\
	oldval = csr_read_allowed(__csr, (ulong)&trap);			\
	if (!trap.cause) {						\
		if (__rdonly) {						\
			(hfeatures->__field)++;				\
		} else {						\
			csr_write_allowed(__csr, (ulong)&trap, __wrval);\
			if (!trap.cause) {				\
				if (csr_swap(__csr, oldval) == __wrval)	\
					(hfeatures->__field)++;		\
				else					\
					goto __skip;			\
			} else {					\
				goto __skip;				\
			}						\
		}							\
	} else {							\
		goto __skip;						\
	}
#define __check_csr_2(__csr, __rdonly, __wrval, __field, __skip)	\
	__check_csr(__csr + 0, __rdonly, __wrval, __field, __skip)	\
	__check_csr(__csr + 1, __rdonly, __wrval, __field, __skip)
#define __check_csr_4(__csr, __rdonly, __wrval, __field, __skip)	\
	__check_csr_2(__csr + 0, __rdonly, __wrval, __field, __skip)	\
	__check_csr_2(__csr + 2, __rdonly, __wrval, __field, __skip)
#define __check_csr_8(__csr, __rdonly, __wrval, __field, __skip)	\
	__check_csr_4(__csr + 0, __rdonly, __wrval, __field, __skip)	\
	__check_csr_4(__csr + 4, __rdonly, __wrval, __field, __skip)
#define __check_csr_16(__csr, __rdonly, __wrval, __field, __skip)	\
	__check_csr_8(__csr + 0, __rdonly, __wrval, __field, __skip)	\
	__check_csr_8(__csr + 8, __rdonly, __wrval, __field, __skip)
#define __check_csr_32(__csr, __rdonly, __wrval, __field, __skip)	\
	__check_csr_16(__csr + 0, __rdonly, __wrval, __field, __skip)	\
	__check_csr_16(__csr + 16, __rdonly, __wrval, __field, __skip)
#define __check_csr_64(__csr, __rdonly, __wrval, __field, __skip)	\
	__check_csr_32(__csr + 0, __rdonly, __wrval, __field, __skip)	\
	__check_csr_32(__csr + 32, __rdonly, __wrval, __field, __skip)

	/**
	 * Detect the allowed address bits & granularity. At least PMPADDR0
	 * should be implemented.
	 */
	val = hart_pmp_get_allowed_addr();
	if (val) {
		hfeatures->pmp_gran =  1 << (sbi_ffs(val) + 2);
		hfeatures->pmp_addr_bits = sbi_fls(val) + 1;
		/* Detect number of PMP regions. At least PMPADDR0 should be implemented*/
		__check_csr_64(CSR_PMPADDR0, 0, val, pmp_count, __pmp_skip);
	}
__pmp_skip:

	/* Detect number of MHPM counters */
	__check_csr(CSR_MHPMCOUNTER3, 0, 1UL, mhpm_count, __mhpm_skip);
	hfeatures->mhpm_bits = hart_pmu_get_allowed_bits();

	__check_csr_4(CSR_MHPMCOUNTER4, 0, 1UL, mhpm_count, __mhpm_skip);
	__check_csr_8(CSR_MHPMCOUNTER8, 0, 1UL, mhpm_count, __mhpm_skip);
	__check_csr_16(CSR_MHPMCOUNTER16, 0, 1UL, mhpm_count, __mhpm_skip);

	/**
	 * No need to check for MHPMCOUNTERH for RV32 as they are expected to be
	 * implemented if MHPMCOUNTER is implemented.
	 */

__mhpm_skip:

#undef __check_csr_64
#undef __check_csr_32
#undef __check_csr_16
#undef __check_csr_8
#undef __check_csr_4
#undef __check_csr_2
#undef __check_csr

	/* Detect if hart supports Priv v1.10 */
	val = csr_read_allowed(CSR_MCOUNTEREN, (unsigned long)&trap);
	if (!trap.cause)
		hfeatures->priv_version = SBI_HART_PRIV_VER_1_10;

	/* Detect if hart supports Priv v1.11 */
	val = csr_read_allowed(CSR_MCOUNTINHIBIT, (unsigned long)&trap);
	if (!trap.cause &&
	    (hfeatures->priv_version >= SBI_HART_PRIV_VER_1_10))
		hfeatures->priv_version = SBI_HART_PRIV_VER_1_11;

	/* Detect if hart supports Priv v1.12 */
	csr_read_allowed(CSR_MENVCFG, (unsigned long)&trap);
	if (!trap.cause &&
	    (hfeatures->priv_version >= SBI_HART_PRIV_VER_1_11))
		hfeatures->priv_version = SBI_HART_PRIV_VER_1_12;

	/* Counter overflow/filtering is not useful without mcounter/inhibit */
	if (hfeatures->priv_version >= SBI_HART_PRIV_VER_1_12) {
		/* Detect if hart supports sscofpmf */
		csr_read_allowed(CSR_SCOUNTOVF, (unsigned long)&trap);
		if (!trap.cause)
			__sbi_hart_update_extension(hfeatures,
					SBI_HART_EXT_SSCOFPMF, true);
	}

	/* Detect if hart supports time CSR */
	csr_read_allowed(CSR_TIME, (unsigned long)&trap);
	if (!trap.cause)
		__sbi_hart_update_extension(hfeatures,
					SBI_HART_EXT_TIME, true);

	/* Detect if hart has AIA local interrupt CSRs */
	csr_read_allowed(CSR_MTOPI, (unsigned long)&trap);
	if (!trap.cause)
		__sbi_hart_update_extension(hfeatures,
					SBI_HART_EXT_SMAIA, true);

	/* Detect if hart supports stimecmp CSR(Sstc extension) */
	if (hfeatures->priv_version >= SBI_HART_PRIV_VER_1_12) {
		csr_read_allowed(CSR_STIMECMP, (unsigned long)&trap);
		if (!trap.cause)
			__sbi_hart_update_extension(hfeatures,
					SBI_HART_EXT_SSTC, true);
	}

	/* Detect if hart supports mstateen CSRs */
	if (hfeatures->priv_version >= SBI_HART_PRIV_VER_1_12) {
		val = csr_read_allowed(CSR_MSTATEEN0, (unsigned long)&trap);
		if (!trap.cause)
			__sbi_hart_update_extension(hfeatures,
					SBI_HART_EXT_SMSTATEEN, true);
	}

	/* Let platform populate extensions */
	rc = sbi_platform_extensions_init(sbi_platform_thishart_ptr(),
					  hfeatures);
	if (rc)
		return rc;

	/* Mark hart feature detection done */
	hfeatures->detected = true;

	return 0;
}

int sbi_hart_reinit(struct sbi_scratch *scratch)
{
	int rc;

	mstatus_init(scratch);

	rc = fp_init(scratch);
	if (rc)
		return rc;

	rc = delegate_traps(scratch);
	if (rc)
		return rc;

	return 0;
}

int sbi_hart_init(struct sbi_scratch *scratch, bool cold_boot)
{
	int rc;

	if (cold_boot) {
		if (misa_extension('H'))
			sbi_hart_expected_trap = &__sbi_expected_trap_hext;

		hart_features_offset = sbi_scratch_alloc_offset(
					sizeof(struct sbi_hart_features));
		if (!hart_features_offset)
			return SBI_ENOMEM;
	}

	rc = hart_detect_features(scratch);
	if (rc)
		return rc;

	return sbi_hart_reinit(scratch);
}

void __attribute__((noreturn)) sbi_hart_hang(void)
{
	while (1)
		wfi();
	__builtin_unreachable();
}

void __attribute__((noreturn))
sbi_hart_switch_mode(unsigned long arg0, unsigned long arg1,
		     unsigned long next_addr, unsigned long next_mode,
		     bool next_virt)
{
#if __riscv_xlen == 32
	unsigned long val, valH;
#else
	unsigned long val;
#endif

	switch (next_mode) {
	case PRV_M:
		break;
	case PRV_S:
		if (!misa_extension('S'))
			sbi_hart_hang();
		break;
	case PRV_U:
		if (!misa_extension('U'))
			sbi_hart_hang();
		break;
	default:
		sbi_hart_hang();
	}

	val = csr_read(CSR_MSTATUS);
	val = INSERT_FIELD(val, MSTATUS_MPP, next_mode);
	val = INSERT_FIELD(val, MSTATUS_MPIE, 0);
#if __riscv_xlen == 32
	if (misa_extension('H')) {
		valH = csr_read(CSR_MSTATUSH);
		valH = INSERT_FIELD(valH, MSTATUSH_MPV, next_virt);
		csr_write(CSR_MSTATUSH, valH);
	}
#else
	if (misa_extension('H'))
		val = INSERT_FIELD(val, MSTATUS_MPV, next_virt);
#endif
	csr_write(CSR_MSTATUS, val);
	csr_write(CSR_MEPC, next_addr);

	if (next_mode == PRV_S) {
		csr_write(CSR_STVEC, next_addr);
		csr_write(CSR_SSCRATCH, 0);
		csr_write(CSR_SIE, 0);
		csr_write(CSR_SATP, 0);
	} else if (next_mode == PRV_U) {
		if (misa_extension('N')) {
			csr_write(CSR_UTVEC, next_addr);
			csr_write(CSR_USCRATCH, 0);
			csr_write(CSR_UIE, 0);
		}
	}

	register unsigned long a0 asm("a0") = arg0;
	register unsigned long a1 asm("a1") = arg1;
	__asm__ __volatile__("mret" : : "r"(a0), "r"(a1));
	__builtin_unreachable();
}
