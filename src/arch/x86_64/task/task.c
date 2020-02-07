
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// task.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "string.h"
#include "assert.h"
#include "cpu.hpp"
#include "gdt/include/gdt.h"
#include "mem/pmm.h"
#include "heap/heap.h"
#include "task/task.h"
#include "include/linkedlist.h"

// 当前任务数量
static uint32_t curr_tasks = 0;
// 全局 pid 值
static pid_t curr_pid = 0;
// 内核进程指针
task_pcb_t kernel_task;
// 任务链表
ListEntry * task_list = NULL;

// 返回一个空的任务控制块
static task_pcb_t * alloc_task_pcb(void) {
	printk_debug("----alloc_task_pcb start----\n");
	// 申请内存
	task_pcb_t * task_pcb = NULL;
	task_pcb = (task_pcb_t *)kmalloc(sizeof(task_pcb_t) );
	if(task_pcb == NULL) {
		printk_err("Error at task.c: alloc_task_pcb. No enough memory!\n");
		return NULL;
	}
	bzero(task_pcb, sizeof(task_pcb_t) );
	// 填充
	task_pcb->status = TASK_UNINIT;
	task_pcb->pid = ++curr_pid;
	char * name = (char *)kmalloc(TASK_NAME_MAX + 1);
	task_pcb->name = name;
	task_pcb->run_time = 0;
	task_pcb->parent = NULL;
	task_mem_t * mm = (task_mem_t *)kmalloc(sizeof(task_mem_t) );
	bzero(mm, sizeof(task_mem_t) );
	task_pcb->mm = mm;
	ptr_t stack = (ptr_t)kmalloc(TASK_STACK_SIZE);
	bzero( (void *)stack, TASK_STACK_SIZE);
	task_pcb->mm->stack = stack + TASK_STACK_SIZE;
	pt_regs_t * pt_regs = (pt_regs_t *)kmalloc(sizeof(pt_regs_t) );
	bzero(pt_regs, sizeof(pt_regs_t) );
	task_pcb->pt_regs = pt_regs;
	task_context_t * context = (task_context_t *)kmalloc(sizeof(task_context_t) );
	bzero(context, sizeof(task_context_t) );
	task_pcb->context = context;
	task_pcb->exit_code = 0;
	return task_pcb;
}

void ttt2(void * arg) {
	printk_test("safafa111----------\n");
	return;
}
extern void switch_to(task_context_t * * prev, task_context_t * * curr);
void task_init(void) {
	cpu_cli();
	kernel_task.status = TASK_RUNNING;
	kernel_task.pid = 1;

	char * name = (char *)kmalloc(TASK_NAME_MAX + 1);
	kernel_task.name = name;
	strcpy(kernel_task.name, "Kernel task");

	kernel_task.run_time = 0;
	kernel_task.parent = NULL;

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

	curr_pid = 1;
	curr_tasks = 1;
	list_append(&task_list, &kernel_task);
	printk_info("task_init\n");
	printk_debug("kernel_task addr: 0x%08X\n", &kernel_task);
	cpu_sti();

	// pt_regs_t pt_regs;
	// bzero(&pt_regs, sizeof(pt_regs_t) );
	// pt_regs.cs = KERNEL_CS;
	// pt_regs.ds = KERNEL_DS;
	// pt_regs.es = KERNEL_CS;
	// pt_regs.user_ss = KERNEL_DS;
	// pt_regs.ebx = (ptr_t)ttt2;
	// pt_regs.edx = (ptr_t)NULL;
	// pt_regs.eip = (uint32_t)kthread_entry;
	task_pcb_t * task = alloc_task_pcb();
	printk_debug("do_fork task addr: 0x%08X\n", task);
	if(task == NULL) {
		printk_err("Error at task.c: do_fork. No enough memory.\n");
		return;
	}
	// 将任务控制结构体放在任务栈的顶部
	task->pt_regs = (pt_regs_t *)( (ptr_t)task->mm->stack - sizeof(pt_regs_t) );
	printk_debug("&task->pt_regs: 0x%08X\n", &task->pt_regs);
	task->pt_regs->cs = KERNEL_CS;
	task->pt_regs->ds = KERNEL_DS;
	task->pt_regs->es = KERNEL_CS;
	task->pt_regs->user_ss = KERNEL_DS;
	task->pt_regs->ebx = (ptr_t)ttt2;
	task->pt_regs->edx = (ptr_t)NULL;
	task->pt_regs->eip = (uint32_t)kthread_entry;
	task->pt_regs->eax = 0;
	task->pt_regs->user_esp = (ptr_t)task->mm->stack - sizeof(pt_regs_t);
	printk_debug("task->pt_regs->user_esp: 0x%08X\n", task->pt_regs->user_esp);
	task->pt_regs->eflags |= EFLAGS_IF;
	// task->context->eip = (uint32_t)forkret_s;
	// task->context->esp = (uint32_t)task->pt_regs;
	task->status = TASK_RUNNABLE;
	task->pid = ++curr_pid;
	task_pcb_t * res = list_append(&task_list, task);
	printk_debug("res: 0x%08X\n", res);

	printk_debug("list_length: 0x%08X\n", list_length(task_list) );
	curr_tasks++;
	printk_debug("before switch_to\n");
	switch_to(&(kernel_task.context), &(task->context) );

	return;
}

int32_t kernel_thread(int32_t (* fun)(void *), void * args, uint32_t flags) {
	printk_debug("----kernel_thread start----\n");
	pt_regs_t pt_regs;
	bzero(&pt_regs, sizeof(pt_regs_t) );
	pt_regs.cs = KERNEL_CS;
	pt_regs.ds = KERNEL_DS;
	pt_regs.es = KERNEL_CS;
	pt_regs.user_ss = KERNEL_DS;
	pt_regs.ebx = (ptr_t)fun;
	pt_regs.edx = (ptr_t)args;
	pt_regs.eip = (uint32_t)kthread_entry;

	return do_fork(&pt_regs, flags);
}

pid_t do_fork(pt_regs_t * pt_regs, uint32_t flags) {
	// 如果到达上限则错误
	if(curr_tasks >= TASK_MAX) {
		printk_err("Error at task.c: do_fork. TASK_MAX!\n");
		return -1;
	}
	task_pcb_t * task = alloc_task_pcb();
	printk_debug("do_fork task addr: 0x%08X\n", task);
	if(task == NULL) {
		printk_err("Error at task.c: do_fork. No enough memory.\n");
		return -1;
	}
	// 将任务控制结构体放在任务栈的顶部
	task->pt_regs = (pt_regs_t *)( (ptr_t)task->mm->stack - sizeof(pt_regs_t) );
	*(task->pt_regs) = *pt_regs;
	task->pt_regs->eax = 0;
	task->pt_regs->user_esp = (ptr_t)task->mm->stack;
	task->pt_regs->eflags |= EFLAGS_IF;
	task->context->eip = (uint32_t)forkret_s;
	// task->context->esp = (uint32_t)task->pt_regs;
	task->status = TASK_RUNNABLE;
	task->pid = ++curr_pid;
	list_append(&task_list, &task);
	curr_tasks++;

	return task->pid;
}

void do_exit(int32_t exit_code) {
	get_current_task()->status = TASK_ZOMBIE;
	get_current_task()->exit_code = exit_code;
	curr_pid--;
	curr_tasks--;
}

task_pcb_t * get_current_task() {
	register uint32_t esp __asm__ ("esp");
	return (task_pcb_t *)(esp & (~(STACK_SIZE - 1) ) );
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
