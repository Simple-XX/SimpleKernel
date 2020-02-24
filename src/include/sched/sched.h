
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
extern void switch_to(task_pcb_t * curr, task_pcb_t * next, task_pcb_t * last);

// 初始化任务调度
void sched_init(void);

// 任务切换
void sched_switch(task_context_t * curr, task_context_t * next);

// 任务调度
void schedule(void);

// 时钟中断处理
void clock_handler(pt_regs_t * regs);

// task_pcb_t * switch_to(task_pcb_t * curr, task_pcb_t * next, task_pcb_t * last);
#define switch_to(prev, next, last) \
	do { \
		uint32_t ebx, ecx, edx, esi, edi; \
		__asm__ volatile ( \
		"pushfl\n\t" \
		"pushl %%ebp\n\t" \
		"movl %%esp,%[prev_sp]\n\t" \
		"movl %[next_sp],%%esp\n\t" \
		"movl $1f,%[prev_ip]\n\t" \
		"pushl %[next_ip]\n\t" \
		"jmp __switch_to\n" \
		"hlt\n\t" \
		"1:\n\t" \
		"popl %%ebp\n\t" \
		"popfl\n" \
		:[prev_sp] "=m" (prev->context->esp), \
		[prev_ip] "=m" (prev->context->eip), \
		"=a" (last), \
		"=b" (ebx), "=c" (ecx), "=d" (edx), \
		"=S" (esi), "=D" (edi) \
		:[next_sp]  "m" (next->context->esp), \
		[next_ip]  "m" (next->context->eip), \
		[prev]     "a" (prev), \
		[next]     "d" (next) \
		: "memory"); \
	} while(0);

 #ifdef __cplusplus
}
 #endif

 #endif /* _SCHED_H_ */
