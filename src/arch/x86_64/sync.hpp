
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// sync.hpp for MRNIU/SimpleKernel.

#ifndef _SYNC_HPP_
#define _SYNC_HPP_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdbool.h"
#include "cpu.hpp"

// 若当前允许中断，则变为禁止
static inline bool intr_store(void) {
	if(EFLAGS_IF_status() ) {
		cpu_cli();
		return true;
	}
	return false;
}

static inline void intr_restore(bool flag) {
	if(flag) {
		cpu_sti();
	}
}

#define local_intr_store(x)     do { x = intr_store(); } while (0)
#define local_intr_restore(x)   intr_restore(x);

#ifdef __cplusplus
}
#endif

#endif /* _SYNC_HPP_ */
