/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Atish Patra <atish.patra@wdc.com>
 */

#ifndef __SBI_BITOPS_H__
#define __SBI_BITOPS_H__

#include <sbi/sbi_types.h>

#if __SIZEOF_POINTER__ == 8
#define BITS_PER_LONG		64
#elif __SIZEOF_POINTER__ == 4
#define BITS_PER_LONG		32
#else
#error "Unexpected __SIZEOF_POINTER__"
#endif

#define EXTRACT_FIELD(val, which) \
	(((val) & (which)) / ((which) & ~((which)-1)))
#define INSERT_FIELD(val, which, fieldval) \
	(((val) & ~(which)) | ((fieldval) * ((which) & ~((which)-1))))

#define BITS_TO_LONGS(nbits)	(((nbits) + BITS_PER_LONG - 1) / \
				 BITS_PER_LONG)

#define BIT(nr)			(1UL << (nr))
#define BIT_MASK(nr)		(1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(bit)		((bit) / BITS_PER_LONG)
#define BIT_WORD_OFFSET(bit)	((bit) & (BITS_PER_LONG - 1))

#define GENMASK(h, l) \
	(((~0UL) - (1UL << (l)) + 1) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

/**
 * sbi_ffs - find first (less-significant) set bit in a long word.
 * @word: The word to search
 *
 * Undefined if no bit exists, so code should check against 0 first.
 */
static inline int sbi_ffs(unsigned long word)
{
	int num = 0;

#if BITS_PER_LONG == 64
	if ((word & 0xffffffff) == 0) {
		num += 32;
		word >>= 32;
	}
#endif
	if ((word & 0xffff) == 0) {
		num += 16;
		word >>= 16;
	}
	if ((word & 0xff) == 0) {
		num += 8;
		word >>= 8;
	}
	if ((word & 0xf) == 0) {
		num += 4;
		word >>= 4;
	}
	if ((word & 0x3) == 0) {
		num += 2;
		word >>= 2;
	}
	if ((word & 0x1) == 0)
		num += 1;
	return num;
}

/*
 * sbi_ffz - find first zero in word.
 * @word: The word to search
 *
 * Undefined if no zero exists, so code should check against ~0UL first.
 */
#define sbi_ffz(x) sbi_ffs(~(x))

/**
 * sbi_fls - find last (most-significant) set bit in a long word
 * @word: the word to search
 *
 * Undefined if no set bit exists, so code should check against 0 first.
 */
static inline unsigned long sbi_fls(unsigned long word)
{
	int num = BITS_PER_LONG - 1;

#if BITS_PER_LONG == 64
	if (!(word & (~0ul << 32))) {
		num -= 32;
		word <<= 32;
	}
#endif
	if (!(word & (~0ul << (BITS_PER_LONG-16)))) {
		num -= 16;
		word <<= 16;
	}
	if (!(word & (~0ul << (BITS_PER_LONG-8)))) {
		num -= 8;
		word <<= 8;
	}
	if (!(word & (~0ul << (BITS_PER_LONG-4)))) {
		num -= 4;
		word <<= 4;
	}
	if (!(word & (~0ul << (BITS_PER_LONG-2)))) {
		num -= 2;
		word <<= 2;
	}
	if (!(word & (~0ul << (BITS_PER_LONG-1))))
		num -= 1;
	return num;
}

#define for_each_set_bit(bit, addr, size) \
	for ((bit) = find_first_bit((addr), (size));		\
	     (bit) < (size);					\
	     (bit) = find_next_bit((addr), (size), (bit) + 1))

/* same as for_each_set_bit() but use bit as value to start with */
#define for_each_set_bit_from(bit, addr, size) \
	for ((bit) = find_next_bit((addr), (size), (bit));	\
	     (bit) < (size);					\
	     (bit) = find_next_bit((addr), (size), (bit) + 1))

#define for_each_clear_bit(bit, addr, size) \
	for ((bit) = find_first_zero_bit((addr), (size));	\
	     (bit) < (size);					\
	     (bit) = find_next_zero_bit((addr), (size), (bit) + 1))

/* same as for_each_clear_bit() but use bit as value to start with */
#define for_each_clear_bit_from(bit, addr, size) \
	for ((bit) = find_next_zero_bit((addr), (size), (bit));	\
	     (bit) < (size);					\
	     (bit) = find_next_zero_bit((addr), (size), (bit) + 1))

unsigned long find_first_bit(const unsigned long *addr,
			     unsigned long size);

unsigned long find_first_zero_bit(const unsigned long *addr,
				  unsigned long size);

unsigned long find_last_bit(const unsigned long *addr,
			    unsigned long size);

unsigned long find_next_bit(const unsigned long *addr,
			    unsigned long size, unsigned long offset);

unsigned long find_next_zero_bit(const unsigned long *addr,
				 unsigned long size,
				 unsigned long offset);

/**
 * __set_bit - Set a bit in memory
 * @nr: the bit to set
 * @addr: the address to start counting from
 *
 * This function is non-atomic and may be reordered.
 */
static inline void __set_bit(int nr, volatile unsigned long *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);

	*p |= mask;
}

/**
 * __clear_bit - Clear a bit in memory
 * @nr: the bit to clear
 * @addr: the address to start counting from
 *
 * This function is non-atomic and may be reordered.
 */
static inline void __clear_bit(int nr, volatile unsigned long *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);

	*p &= ~mask;
}

/**
 * __change_bit - Toggle a bit in memory
 * @nr: the bit to change
 * @addr: the address to start counting from
 *
 * This function is non-atomic and may be reordered.
 */
static inline void __change_bit(int nr, volatile unsigned long *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);

	*p ^= mask;
}

/**
 * __test_and_set_bit - Set a bit and return its old value
 * @nr: Bit to set
 * @addr: Address to count from
 *
 * This operation is non-atomic and can be reordered.
 */
static inline int __test_and_set_bit(int nr, volatile unsigned long *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);
	unsigned long old = *p;

	*p = old | mask;
	return (old & mask) != 0;
}

/**
 * __test_and_clear_bit - Clear a bit and return its old value
 * @nr: Bit to clear
 * @addr: Address to count from
 *
 * This operation is non-atomic and can be reordered.
 */
static inline int __test_and_clear_bit(int nr, volatile unsigned long *addr)
{
	unsigned long mask = BIT_MASK(nr);
	unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);
	unsigned long old = *p;

	*p = old & ~mask;
	return (old & mask) != 0;
}

/**
 * __test_bit - Determine whether a bit is set
 * @nr: bit number to test
 * @addr: Address to start counting from
 *
 * This operation is non-atomic and can be reordered.
 */
static inline int __test_bit(int nr, const volatile unsigned long *addr)
{
	return 1UL & (addr[BIT_WORD(nr)] >> (nr & (BITS_PER_LONG-1)));
}

#endif
