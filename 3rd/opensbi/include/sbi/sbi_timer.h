/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#ifndef __SBI_TIMER_H__
#define __SBI_TIMER_H__

#include <sbi/sbi_types.h>

/** Timer hardware device */
struct sbi_timer_device {
	/** Name of the timer operations */
	char name[32];

	/** Frequency of timer in HZ */
	unsigned long timer_freq;

	/** Get free-running timer value */
	u64 (*timer_value)(void);

	/** Start timer event for current HART */
	void (*timer_event_start)(u64 next_event);

	/** Stop timer event for current HART */
	void (*timer_event_stop)(void);
};

struct sbi_scratch;

/** Generic delay loop of desired granularity */
void sbi_timer_delay_loop(ulong units, u64 unit_freq,
			  void (*delay_fn)(void *), void *opaque);

/** Provide delay in terms of milliseconds */
static inline void sbi_timer_mdelay(ulong msecs)
{
	sbi_timer_delay_loop(msecs, 1000, NULL, NULL);
}

/** Provide delay in terms of microseconds */
static inline void sbi_timer_udelay(ulong usecs)
{
	sbi_timer_delay_loop(usecs, 1000000, NULL, NULL);
}

/**
 * A blocking function that will wait until @p predicate returns true or
 * @p timeout_ms milliseconds elapsed. @p arg will be passed as argument to
 * @p predicate function.
 *
 * @param predicate Pointer to a function that returns true if certain
 * condition is met. It shouldn't block the code execution.
 * @param arg Argument to pass to @p predicate.
 * @param timeout_ms Timeout value in milliseconds. The function will return
 * false if @p timeout_ms time period elapsed but still @p predicate doesn't
 * return true.
 *
 * @return true if @p predicate returns true within @p timeout_ms, false
 * otherwise.
 */
bool sbi_timer_waitms_until(bool (*predicate)(void *), void *arg,
			    uint64_t timeout_ms);

/** Get timer value for current HART */
u64 sbi_timer_value(void);

/** Get virtualized timer value for current HART */
u64 sbi_timer_virt_value(void);

/** Get timer delta value for current HART */
u64 sbi_timer_get_delta(void);

/** Set timer delta value for current HART */
void sbi_timer_set_delta(ulong delta);

/** Set upper 32-bits of timer delta value for current HART */
void sbi_timer_set_delta_upper(ulong delta_upper);

/** Start timer event for current HART */
void sbi_timer_event_start(u64 next_event);

/** Process timer event for current HART */
void sbi_timer_process(void);

/** Get current timer device */
const struct sbi_timer_device *sbi_timer_get_device(void);

/** Register timer device */
void sbi_timer_set_device(const struct sbi_timer_device *dev);

/* Initialize timer */
int sbi_timer_init(struct sbi_scratch *scratch, bool cold_boot);

/* Exit timer */
void sbi_timer_exit(struct sbi_scratch *scratch);

#endif
