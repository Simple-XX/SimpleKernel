
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

// 可调度进程链表
extern task_pcb_t * running_proc_head;

// 等待进程链表
extern task_pcb_t * wait_proc_head;

// 当前运行的任务
extern task_pcb_t * current;

// 上下文切换
extern void switch_to(task_context_t * * prev, task_context_t * * curr);

// 初始化任务调度
void sched_init(void);

// 任务切换
void sched_switch(task_context_t * curr, task_context_t * next);

// 任务调度
void schedule(void);







#ifdef __cplusplus
}
#endif

#endif /* _SCHED_H_ */
