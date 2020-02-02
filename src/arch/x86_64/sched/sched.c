
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// sched.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "sched/sched.h"

// 可调度进程链表
task_pcb_t * running_proc_head = NULL;

// 等待进程链表
task_pcb_t * wait_proc_head = NULL;

// 当前运行的任务
task_pcb_t * current = NULL;

void sched_init() {
	return;
}

void sched_switch(task_context_t * curr UNUSED, task_context_t * next UNUSED) {
	return;
}

void schedule() {
	return;
}

#ifdef __cplusplus
}
#endif
