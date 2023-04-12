/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2021 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Atish Patra <atish.patra@wdc.com>
 */

#ifndef __SBI_PMU_H__
#define __SBI_PMU_H__

#include <sbi/sbi_types.h>

struct sbi_scratch;

/* Event related macros */
/* Maximum number of hardware events that can mapped by OpenSBI */
#define SBI_PMU_HW_EVENT_MAX 256

/* Counter related macros */
#define SBI_PMU_FW_CTR_MAX 16
#define SBI_PMU_HW_CTR_MAX 32
#define SBI_PMU_CTR_MAX	   (SBI_PMU_HW_CTR_MAX + SBI_PMU_FW_CTR_MAX)
#define SBI_PMU_FIXED_CTR_MASK 0x07

struct sbi_pmu_device {
	/** Name of the PMU platform device */
	char name[32];

	/**
	 * Validate event code of custom firmware event
	 * Note: SBI_PMU_FW_MAX <= event_idx_code
	 */
	int (*fw_event_validate_code)(uint32_t event_idx_code);

	/**
	 * Match custom firmware counter with custom firmware event
	 * Note: 0 <= counter_index < SBI_PMU_FW_CTR_MAX
	 */
	bool (*fw_counter_match_code)(uint32_t counter_index,
				      uint32_t event_idx_code);

	/**
	 * Read value of custom firmware counter
	 * Note: 0 <= counter_index < SBI_PMU_FW_CTR_MAX
	 */
	uint64_t (*fw_counter_read_value)(uint32_t counter_index);

	/**
	 * Start custom firmware counter
	 * Note: SBI_PMU_FW_MAX <= event_idx_code
	 * Note: 0 <= counter_index < SBI_PMU_FW_CTR_MAX
	 */
	int (*fw_counter_start)(uint32_t counter_index,
				uint32_t event_idx_code,
				uint64_t init_val, bool init_val_update);

	/**
	 * Stop custom firmware counter
	 * Note: 0 <= counter_index < SBI_PMU_FW_CTR_MAX
	 */
	int (*fw_counter_stop)(uint32_t counter_index);

	/**
	 * Custom enable irq for hardware counter
	 * Note: 0 <= counter_index < SBI_PMU_HW_CTR_MAX
	 */
	void (*hw_counter_enable_irq)(uint32_t counter_index);

	/**
	 * Custom disable irq for hardware counter
	 * Note: 0 <= counter_index < SBI_PMU_HW_CTR_MAX
	 */
	void (*hw_counter_disable_irq)(uint32_t counter_index);

	/**
	 * Custom function returning the machine-specific irq-bit.
	 */
	int (*hw_counter_irq_bit)(void);
};

/** Get the PMU platform device */
const struct sbi_pmu_device *sbi_pmu_get_device(void);

/** Set the PMU platform device */
void sbi_pmu_set_device(const struct sbi_pmu_device *dev);

/** Initialize PMU */
int sbi_pmu_init(struct sbi_scratch *scratch, bool cold_boot);

/** Reset PMU during hart exit */
void sbi_pmu_exit(struct sbi_scratch *scratch);

/** Return the pmu irq bit depending on extension existence */
int sbi_pmu_irq_bit(void);

/**
 * Add the hardware event to counter mapping information. This should be called
 * from the platform code to update the mapping table.
 * @param eidx_start Start of the event idx range for supported counters
 * @param eidx_end   End of the event idx range for supported counters
 * @param cmap       A bitmap representing counters supporting the event range
 * @return 0 on success, error otherwise.
 */
int sbi_pmu_add_hw_event_counter_map(u32 eidx_start, u32 eidx_end, u32 cmap);

/**
 * Add the raw hardware event selector and supported counter information. This
 * should be called from the platform code to update the mapping table.
 * @param info  a pointer to the hardware event info
 * @return 0 on success, error otherwise.
 */

int sbi_pmu_add_raw_event_counter_map(uint64_t select, uint64_t select_mask, u32 cmap);

int sbi_pmu_ctr_fw_read(uint32_t cidx, uint64_t *cval);

int sbi_pmu_ctr_stop(unsigned long cidx_base, unsigned long cidx_mask,
		     unsigned long flag);

int sbi_pmu_ctr_start(unsigned long cidx_base, unsigned long cidx_mask,
		      unsigned long flags, uint64_t ival);

int sbi_pmu_ctr_get_info(uint32_t cidx, unsigned long *ctr_info);

unsigned long sbi_pmu_num_ctr(void);

int sbi_pmu_ctr_cfg_match(unsigned long cidx_base, unsigned long cidx_mask,
			  unsigned long flags, unsigned long event_idx,
			  uint64_t event_data);

int sbi_pmu_ctr_incr_fw(enum sbi_pmu_fw_event_code_id fw_id);

#endif
