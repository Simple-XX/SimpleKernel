
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
extern void switch_to(task_context_t * curr, task_context_t * next);

// 初始化任务调度
void sched_init(void);

// 任务切换
void sched_switch(task_context_t * curr, task_context_t * next);

// 任务调度
void schedule(void);

// 时钟中断处理
void clock_handler(pt_regs_t * regs);

/*
 #define SWITCH_TO(curr, next) { \
                __asm__ volatile ( \
                "mov %[curr], %[curr_eip]\n\t" \
                "mov %%esp, %[curr_esp]\n\t" \
                "mov %%ebp, %[curr_ebp]\n\t" \
                "mov %%ebx, %[curr_ebx]\n\t" \
                "mov %%ecx, %[curr_ecx]\n\t" \
                "mov %%edx, %[curr_edx]\n\t" \
                "mov %%esi, %[curr_esi]\n\t" \
                "mov %%edi, %[curr_edi]\n" \
                "mov %[next_eip], %%eip\n\t" \
                "mov %[next_esp], %%esp\n\t" \
                "mov %[next_ebp], %%ebp\n\t" \
                "mov %[next_ebx], %%ebx\n\t" \
                "mov %[next_ecx], %%ecx\n\t" \
                "mov %[next_edx], %%edx\n\t" \
                "mov %[next_esi], %%esi\n\t" \
                "mov %[next_edi], %%edi\n" \
                :[curr_eip] "=m" (curr->eip), \
                [curr_esp] "=m" (curr->esp), \
                [curr_ebp] "=m" (curr->ebp), \
                [curr_ebx] "=m" (curr->ebx), \
                [curr_ecx] "=m" (curr->ecx), \
                [curr_edx] "=m" (curr->edx), \
                [curr_esi] "=m" (curr->esi), \
                [curr_edi] "=m" (curr->edi) \
                :[next_eip] "m" (next->eip), \
                [next_esp] "m" (next->esp), \
                [next_ebp] "m" (next->ebp), \
                [next_ebx] "m" (next->ebx), \
                [next_ecx] "m" (next->ecx), \
                [next_edx] "m" (next->edx), \
                [next_esi] "m" (next->esi), \
                [next_edi] "m" (next->edi) \
                : "memory"); \
   }

   inline void __switch_to(task_pcb_t * curr, task_pcb_t * next) {
        return;
   }

   // void SWITCH_TO(task_pcb_t * curr, task_pcb_t * next)
 #define SWITCH_TO(curr, next) \
        do { \
                __asm__ volatile ( \
                "push %%ebp\n\t" \
                "push %%eax\n\t" \
                "mov %%esp, %0\n\t" \
                "mov %2, %%esp\n\t" \
                "lea 1f(%%eip), %%eax\n\t" \
                "mov %%eax, %1\n\t" \
                "push %3\n\t" \
                "jmp __switch_to\n\t" \
                "1:\n\t" \
                "pop %%eax\n\t" \
                "pop %%ebp\n\t" \
                : "=m" (curr->pt_regs->user_esp), \
                "=m" (curr->pt_regs->eip) \
                : "m" (next->pt_regs->user_esp), \
                "m" (next->pt_regs->eip), \
                "D" (curr), \
                "S" (next), \
                : "memory"); \
        } while(0);


 */

 #ifdef __cplusplus
}
 #endif

 #endif /* _SCHED_H_ */
