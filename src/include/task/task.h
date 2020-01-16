
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// task.h for MRNIU/SimpleKernel.

#ifndef _TASK_H_
#define _TASK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

// 进程状态描述
typedef
        enum task_status {
	// 未初始化
	TASK_UNINIT = 0,
	// 睡眠中
	TASK_SLEEPING = 1,
	// 可运行(也许正在运行)
	TASK_RUNNABLE = 2,
	// 僵尸状态
	TASK_ZOMBIE = 3,
} task_status_t;

// 内核线程的上下文切换保存的信息
typedef
        struct task_context {
	uint32_t esp;
	uint32_t ebp;
	uint32_t ebx;
	uint32_t esi;
	uint32_t edi;
	uint32_t eflags;
} task_context_t;

// 进程内存地址结构
typedef
        struct task_mem {
	pgd_t * pgd_dir;     // 进程页表
} task_mem_t;

// 进程控制块 PCB
typedef
        struct task_pcb {
	// 进程当前状态
	volatile task_status_t state;
	// 进程标识符
	pid_t pid;
	// 进程的内核栈地址
	void    * stack;
	// 当前进程的内存地址映像
	task_mem_t * mm;
	// 进程切换需要的上下文信息
	task_context_t context;
	// 链表指针
	struct task_pcb * next;
} task_pcb_t;

// 全局 pid 值
extern pid_t curr_pid;

// 内核线程创建
// 线程退出函数

void task_init(void);

#ifdef __cplusplus
}
#endif

#endif /* _TASK_H_ */
