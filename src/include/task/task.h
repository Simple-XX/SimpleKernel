
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// task.h for MRNIU/SimpleKernel.

#ifndef _TASK_H_
#define _TASK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "types.h"
#include "mem/vmm.h"
#include "intr/include/intr.h"

// 最长任务名
#define TASK_NAME_MAX 64

// 进程状态描述
typedef
    enum task_status {
	// 未初始化
	TASK_UNINIT = 0,
	// 睡眠中
	TASK_SLEEPING = 1,
	// 可运行
	TASK_RUNNABLE = 2,
	// 正在运行
	TASK_RUNNING = 3,
	// 僵尸状态
	TASK_ZOMBIE = 4,
} task_status_t;

// 内核线程的上下文切换保存的信息
typedef
    struct task_context {
	uint32_t	edi;
	uint32_t	esi;
	uint32_t	ebx;
	uint32_t	ebp;
	uint32_t	eip;
} task_context_t;

// 进程内存地址结构
typedef
    struct task_mem {
	pgd_t *      pgd_dir;     // 进程页表
} task_mem_t;

// 进程控制块 PCB
typedef
    struct task_pcb {
	// 任务状态
	volatile task_status_t        status;
	// 任务的内核栈指针
	ptr_t			stack;
	// 任务的 pid
	pid_t			pid;
	// 任务名称
	char *			name;
	// 当前任务运行时间
	uint32_t		run_time;
	// 父进程指针
	struct task_pcb *	parent;
	// 任务的内存信息
	task_mem_t		mm;
	// 任务中断保存的寄存器信息
	pt_regs_t *		pt_regs;
	// 任务切换上下文信息
	task_context_t *	context;
	// 任务的退出代码
	int32_t			exit_code;
	// 链表指针
	struct task_pcb *	next;
} task_pcb_t;

// 全局 pid 值
extern pid_t curr_pid;
// 初始化
void task_init(void);
// 线程创建
pid_t kfork(int (* fn)(void *), void * arg);
// 线程退出
void kexit(void);

int32_t kexec();
int32_t kwait();
void kwakeup();
void kkill();

// 任务切换宏
// void SWITCH_TO(task_context_t * old, task_context_t * new);
#define SWITCH_TO(curr, next) \
	{ \
		__asm__ volatile ( \
		/*  */             \
		"mov 4(%esp), %eax\n\t" \
		"mov 8(%esp), %edx\n\t" \
		/*  */                        \
		"push %ebp\n\t" \
		"push %ebx\n\t" \
		"push %esi\n\t" \
		"push %edi\n\t" \
		/*  */        \
		"mov %esp, (%eax)\n\t" \
		"mov (%edx), %esp\n\t" \
		/*  */           \
		"pop %edi\n\t" \
		"pop %esi\n\t" \
		"pop %ebx\n\t" \
		"pop %ebp\n\t" \
		"ret" \
		); \
	}

#ifdef __cplusplus
}
#endif

#endif /* _TASK_H_ */
