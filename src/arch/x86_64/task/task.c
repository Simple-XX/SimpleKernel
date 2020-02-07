
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// task.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "string.h"
#include "assert.h"
#include "cpu.hpp"
#include "mem/pmm.h"
#include "heap/heap.h"
#include "task/task.h"
#include "include/linkedlist.h"

// 全局 pid 值
pid_t curr_pid = 0;
// 内核进程指针
static task_pcb_t kernel_task;

// 返回一个空的任务控制块
static task_pcb_t * alloc_task_pcb(void) {
	// 申请内存
	task_pcb_t * task_pcb;
	task_pcb = (task_pcb_t *)kmalloc(sizeof(task_pcb_t) );
	bzero(task_pcb, sizeof(task_pcb_t) );
	// 填充


	return task_pcb;
}

void task_init(void) {
	cpu_cli();
	kernel_task.status = TASK_RUNNING;
	kernel_task.pid = 1;

	char * name = (char *)kmalloc(TASK_NAME_MAX + 1);
	kernel_task.name = name;
	strcpy(kernel_task.name, "Kernel task");

	kernel_task.run_time = 0;
	kernel_task.parent = &kernel_task;

	task_mem_t * mm = (task_mem_t *)kmalloc(sizeof(task_mem_t) );
	bzero(mm, sizeof(task_mem_t) );
	mm->pgd_dir = pgd_kernel;
	mm->stack = kernel_stack_top;
	mm->task_start = (ptr_t)&kernel_start;
	mm->code_start = (ptr_t)&kernel_text_start;
	mm->code_end = (ptr_t)&kernel_text_end;
	mm->data_start = (ptr_t)&kernel_data_start;
	mm->data_end = (ptr_t)&kernel_data_end;
	mm->task_end = (ptr_t)&kernel_end;
	kernel_task.mm = mm;

	kernel_task.pt_regs = NULL;
	kernel_task.context = NULL;
	kernel_task.exit_code = 0;
	kernel_task.next = NULL;
	printk_info("task_init\n");
	cpu_sti();
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
