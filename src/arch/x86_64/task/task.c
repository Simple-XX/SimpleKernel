
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

// 当前任务数量
static uint32_t curr_task_count = 0;
// 全局 pid 值
static pid_t curr_pid = 0;
// 全部任务链表
ListEntry * task_list = NULL;
// 可调度进程链表
ListEntry * runnable_list = NULL;
// 等待进程链表
ListEntry * wait_list = NULL;

// 返回一个空的任务控制块
static task_pcb_t * alloc_task_pcb(void) {
	// 申请内存
	task_pcb_t * task_pcb = (task_pcb_t *)kmalloc(TASK_STACK_SIZE);
	if(task_pcb == NULL) {
		printk_err("Error at task.c: alloc_task_pcb. No enough memory!\n");
		return NULL;
	}
	bzero(task_pcb, sizeof(task_pcb_t) );
	// 填充
	task_pcb->status = TASK_UNINIT;
	task_pcb->pid = ++curr_pid;
	char * name = (char *)kmalloc(TASK_NAME_MAX + 1);
	bzero(name, TASK_NAME_MAX + 1);
	task_pcb->name = name;
	task_pcb->run_time = 0;
	task_pcb->parent = NULL;
	task_mem_t * mm = (task_mem_t *)kmalloc(sizeof(task_mem_t) );
	bzero(mm, sizeof(task_mem_t) );
	task_pcb->mm = mm;
	// ptr_t stack_bottom = (ptr_t)kmalloc(TASK_STACK_SIZE);
	// bzero( (void *)stack_bottom, TASK_STACK_SIZE);
	task_pcb->mm->stack_top = (ptr_t)task_pcb + TASK_STACK_SIZE;
	task_pcb->mm->stack_bottom = (ptr_t)task_pcb;
	pt_regs_t * pt_regs = (pt_regs_t *)kmalloc(sizeof(pt_regs_t) );
	bzero(pt_regs, sizeof(pt_regs_t) );
	task_pcb->pt_regs = pt_regs;
	task_context_t * context = (task_context_t *)kmalloc(sizeof(task_context_t) );
	bzero(context, sizeof(task_context_t) );
	task_pcb->context = context;
	task_pcb->context->eflags |= EFLAGS_IF;
	task_pcb->exit_code = 0;
	list_append(&task_list, task_pcb);
	curr_task_count++;
	return task_pcb;
}

void task_init(void) {
	cpu_cli();
	// 为内核进程创建信息结构体，位置在内核栈的栈底
	task_pcb_t * kernel_task = (task_pcb_t *)(kernel_stack);
	bzero(kernel_task, sizeof(task_pcb_t) );
	kernel_task->status = TASK_RUNNING;
	kernel_task->pid = 1;

	char * name = (char *)kmalloc(TASK_NAME_MAX + 1);
	bzero(name, TASK_NAME_MAX + 1);
	kernel_task->name = name;
	strcpy(kernel_task->name, "Kernel task");

	kernel_task->run_time = 0;
	kernel_task->parent = NULL;

	task_mem_t * mm = (task_mem_t *)kmalloc(sizeof(task_mem_t) );
	bzero(mm, sizeof(task_mem_t) );
	mm->pgd_dir = pgd_kernel;
	mm->stack_bottom = (ptr_t)kernel_stack;
	mm->stack_top = (ptr_t)kernel_stack + TASK_STACK_SIZE;
	mm->task_start = (ptr_t)&kernel_start;
	mm->code_start = (ptr_t)&kernel_text_start;
	mm->code_end = (ptr_t)&kernel_text_end;
	mm->data_start = (ptr_t)&kernel_data_start;
	mm->data_end = (ptr_t)&kernel_data_end;
	mm->task_end = (ptr_t)&kernel_end;
	kernel_task->mm = mm;

	pt_regs_t * pt_regs = (pt_regs_t *)kmalloc(sizeof(pt_regs_t) );
	bzero(pt_regs, sizeof(pt_regs_t) );
	kernel_task->pt_regs = pt_regs;

	task_context_t * context = (task_context_t *)kmalloc(sizeof(task_context_t) );
	bzero(context, sizeof(task_context_t) );
	kernel_task->context = context;
	kernel_task->context->esp = (ptr_t)kernel_task + TASK_STACK_SIZE;
	printk_debug("kernel_task->context->esp: 0x%08X\n", (kernel_task->context->esp) );
	kernel_task->context->eflags |= EFLAGS_IF;
	kernel_task->exit_code = 0;
	curr_pid = 1;
	curr_task_count = 1;
	list_append(&task_list, kernel_task);
	list_append(&runnable_list, kernel_task);
	printk_info("task_init\n");
	return;
}

int32_t kernel_thread(int32_t (* fun)(void *), void * args, uint32_t flags) {
	task_pcb_t * new_task = alloc_task_pcb();
	// printk_debug("new_task: 0x%08X\n", (new_task) );
	new_task->mm->pgd_dir = pgd_kernel;
	// 处理参数
	ptr_t * stack_top = (ptr_t *)( (ptr_t)new_task + TASK_STACK_SIZE);
	*(--stack_top) = (uint32_t)args;
	*(--stack_top) = (uint32_t)kthread_exit;
	*(--stack_top) = (uint32_t)fun;

	// 指向当前栈的位置
	new_task->context->esp = (ptr_t)new_task + TASK_STACK_SIZE - sizeof(ptr_t) * 3;
	new_task->context->eflags |= EFLAGS_IF;
	new_task->status = TASK_RUNNABLE;
	list_append(&runnable_list, new_task);

	// return do_fork(&pt_regs, flags);
	return new_task->pid;
}

void kthread_exit() {
	register uint32_t val asm ("eax");
	printk("Thread exited with value %d\n", val);
	while(1);
}


pid_t do_fork(pt_regs_t * pt_regs, uint32_t flags) {
	// // 如果到达上限则错误
	// if(curr_task_count >= TASK_MAX) {
	// 	printk_err("Error at task.c: do_fork. TASK_MAX!\n");
	// 	return -1;
	// }
	// task_pcb_t * task = alloc_task_pcb();
	// printk_debug("do_fork task addr: 0x%08X\n", task);
	// if(task == NULL) {
	// 	printk_err("Error at task.c: do_fork. No enough memory.\n");
	// 	return -1;
	// }
	// // 将任务控制结构体放在任务栈的顶部
	// task->pt_regs = (pt_regs_t *)( (ptr_t)task->mm->stack - sizeof(pt_regs_t) );
	// *(task->pt_regs) = *pt_regs;
	// task->pt_regs->eax = 0;
	// task->pt_regs->user_esp = (ptr_t)task->mm->stack;
	// task->pt_regs->eflags |= EFLAGS_IF;
	// task->context->eip = (uint32_t)forkret_s;
	// // task->context->esp = (uint32_t)task->pt_regs;
	// task->status = TASK_RUNNABLE;
	// task->pid = ++curr_pid;
	// list_append(&task_list, &task);
	// curr_task_count++;
	//
	// return task->pid;
}

void do_exit(int32_t exit_code) {
	get_current_task()->status = TASK_ZOMBIE;
	get_current_task()->exit_code = exit_code;
	curr_pid--;
	curr_task_count--;
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
