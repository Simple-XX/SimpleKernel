
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// task.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "string.h"
#include "assert.h"
#include "task/task.h"

// 全局 pid 值
pid_t curr_pid = 0;

void task_init(void) {
	return;
}

// 线程创建
pid_t kfork(int (* fn)(void *), void * arg) {
	return 0;
}

void kexit() {
	return;
}

#ifdef __cplusplus
}
#endif
