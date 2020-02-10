
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// sched.h for MRNIU/SimpleKernel.

#ifndef _SCHED_H_
#define _SCHED_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "task/task.h"

// 上下文切换
extern void switch_to(task_context_t * prev, task_context_t * curr);

// 初始化任务调度
void sched_init(void);

// 任务切换
void sched_switch(task_context_t * curr, task_context_t * next);

// 任务调度
void schedule(void);

// 任务切换准备
void change_task_to(task_pcb_t * next);


#ifdef __cplusplus
}
#endif

#endif /* _SCHED_H_ */
