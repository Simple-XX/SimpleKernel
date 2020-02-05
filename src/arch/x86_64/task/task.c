
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// task.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "string.h"
#include "mem/pmm.h"
#include "assert.h"
#include "heap/heap.h"
#include "task/task.h"
#include "include/linkedlist.h"

// 全局 pid 值
pid_t curr_pid = 0;

// 返回一个空的任务控制块
static task_pcb_t * alloc_task_pcb(void) {
	// 申请内存
	task_pcb_t * task_pcb;
	task_pcb = (task_pcb_t *)kmalloc(sizeof(task_pcb_t) );
	bzero(task_pcb, sizeof(task_pcb_t) );
	// 填充
	task_pcb->status = TASK_UNINIT;
	// 如果是在用户模式下，分配 8KB 栈
	// task_pcb->stack = kmalloc(2 * VMM_PAGE_SIZE);
	// 内核模式下指向内核栈
	task_pcb->stack = (ptr_t)kernel_stack_top;
	task_pcb->pid = -1;
	task_pcb->name = (char *)kmalloc(TASK_NAME_MAX + 1);
	task_pcb->name[0] = 'a';
	task_pcb->name[1] = 'a';
	task_pcb->name[2] = 'a';
	task_pcb->name[3] = 'a';
	task_pcb->name[4] = 'a';
	task_pcb->name[5] = 'a';
	task_pcb->name[6] = '\0';
	task_pcb->run_time = 0;
	task_pcb->parent = NULL;
	task_pcb->mm.pgd_dir = (pgd_t *)kmalloc(sizeof(pgd_t) );
	task_pcb->pt_regs = (pt_regs_t *)kmalloc(sizeof(pt_regs_t) );
	task_pcb->context = (task_context_t *)kmalloc(sizeof(task_context_t) );
	task_pcb->next = NULL;

	return task_pcb;
}

void task_init(void) {
	task_pcb_t * kernel_task = alloc_task_pcb();
	printk_test("name: %s\n", *(kernel_task->name) );
	return;
}

// 线程创建
pid_t kfork(int (* fn)(void *) UNUSED, void * arg UNUSED) {
	return 0;
}

void kexit() {
	return;
}

#ifdef __cplusplus
}
#endif
