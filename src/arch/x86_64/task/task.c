
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

// 当前任务指针
task_pcb_t * curr_task = NULL;
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

// 申请栈空间
static inline ptr_t alloc_stack(void);
static inline ptr_t alloc_stack(void) {

	return 0;
}

// 返回一个空的任务控制块
static task_pcb_t * alloc_task_pcb(void) {
	cpu_cli();
	// 申请内存
	task_pcb_t * task_pcb = (task_pcb_t *)kmalloc(TASK_STACK_SIZE);
	assert(task_pcb != NULL, "Error at task.c: alloc_task_pcb. No enough memory!\n");
	bzero(task_pcb, TASK_STACK_SIZE);
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
	task_pcb->mm->stack_top =  (ptr_t)task_pcb;
	task_pcb->mm->stack_bottom = task_pcb->mm->stack_top + TASK_STACK_SIZE;
	task_pcb->pt_regs = (pt_regs_t *)( (ptr_t)task_pcb->mm->stack_bottom - sizeof(pt_regs_t) );
	bzero(task_pcb->pt_regs, sizeof(pt_regs_t) );
	task_context_t * context = (task_context_t *)kmalloc(sizeof(task_context_t) );
	bzero(context, sizeof(task_context_t) );
	context->eip = (ptr_t)forkret_s233;
	context->esp = (ptr_t)task_pcb->pt_regs;
	task_pcb->context = context;
	task_pcb->exit_code = 0xCD;
	list_append(&task_list, task_pcb);
	// 增加全局进程数量
	curr_task_count++;
	return task_pcb;
}

// 为正在运行的进程初始化控制信息
void task_init(void) {
	cpu_cli();
	// 创建一个新的进程作为内核进程
	task_pcb_t * kernel_task = get_current_task();
	assert(kernel_task == (ptr_t)KERNEL_STACK_TOP, "kernel_task not correct\n");
	bzero(kernel_task, sizeof(task_pcb_t) );
	// 设置进程名
	kernel_task->name = (char *)kmalloc(TASK_NAME_MAX + 1);
	bzero(kernel_task->name, TASK_NAME_MAX + 1);
	strcpy(kernel_task->name, "Kernel task");
	// 设置页目录
	kernel_task->mm = (task_mem_t *)kmalloc(sizeof(task_mem_t) );
	bzero(kernel_task->mm, sizeof(task_mem_t) );
	kernel_task->mm->pgd_dir = pgd_kernel;
	// 设置上下文
	kernel_task->context = (task_context_t *)kmalloc(sizeof(task_context_t) );
	bzero(kernel_task->context, sizeof(task_context_t) );
	// context->eip = (ptr_t)forkret_s233;
	// context->esp = (ptr_t)task_pcb->pt_regs;
	// 设置寄存器信息
	// kernel_task->pt_regs = (pt_regs_t *)kmalloc(sizeof(pt_regs_t) );
	// kernel_task->pt_regs->cs = KERNEL_CS;
	// kernel_task->pt_regs->ds = KERNEL_DS;
	// kernel_task->pt_regs->es = KERNEL_DS;
	// kernel_task->pt_regs->fs = KERNEL_DS;
	// kernel_task->pt_regs->gs = KERNEL_DS;
	// kernel_task->pt_regs->user_ss = KERNEL_DS;
	// kernel_task->pt_regs->eflags |= EFLAGS_IF;
	// kernel_task->pt_regs->eip = (ptr_t)forkret_s233;
	// 设置进程运行状态
	kernel_task->status = TASK_RUNNING;
	curr_task = kernel_task;
	// 添加到可运行/正在运行任链表
	list_append(&runnable_list, kernel_task);
	printk_info("task_init\n");
	return;
}

// 创建内核进程
pid_t kernel_thread(int32_t (* fun)(void *), void * args, uint32_t flags) {
	cpu_cli();
	pt_regs_t pt_regs;
	bzero(&pt_regs, sizeof(pt_regs_t) );
	pt_regs.cs = KERNEL_CS;
	pt_regs.ds = KERNEL_DS;
	pt_regs.es = KERNEL_DS;
	pt_regs.fs = KERNEL_DS;
	pt_regs.gs = KERNEL_DS;
	pt_regs.user_ss = KERNEL_DS;
	pt_regs.ebx = (ptr_t)fun;
	pt_regs.edx = (ptr_t)args;
	pt_regs.eip = (ptr_t)kthread_entry;
	return do_fork(flags, &pt_regs);
}

// 将 pt_reg 拷贝到 PCB 中
static void copy_thread(task_pcb_t * task, pt_regs_t * pt_regs) {
	cpu_cli();
	memcpy(task->pt_regs, pt_regs, sizeof(pt_regs_t) );
	task->pt_regs->eax = 0;
	task->pt_regs->user_esp = (ptr_t)task->mm->stack_bottom;
	task->pt_regs->eflags |= EFLAGS_IF;
	return;
}

// 创建 PCB，设置相关信息后加入调度链表
pid_t do_fork(uint32_t flags, pt_regs_t * pt_regs) {
	cpu_cli();
	assert(curr_task_count < TASK_MAX, "Error: task.c curr_task_count >= TASK_MAX");
	task_pcb_t * task = alloc_task_pcb();
	assert(task != NULL, "Error: task.c task==NULL");
	copy_thread(task, pt_regs);
	task->pid = ++curr_pid;
	task->status = TASK_RUNNABLE;
	list_append(&runnable_list, task);
	cpu_sti();
	return task->pid;
}

void do_exit(int32_t exit_code) {
	cpu_cli();
	// get_current_task()->status = TASK_ZOMBIE;
	// get_current_task()->exit_code = exit_code;
	curr_pid--;
	curr_task_count--;
	print_stack(10);
	// asm ("hlt");
	cpu_sti();
	return;
}

// 获取正在运行进程的控制结构体
task_pcb_t * get_current_task(void) {
	register uint32_t esp __asm__ ("esp");
	return (task_pcb_t *)(esp & (~(TASK_STACK_SIZE - 1) ) );
}

void kthread_exit(void) {
	register uint32_t val __asm__ ("eax");
	printk("Thread exited with value %d\n", val);
	while(1);
	return;
}

task_pcb_t __attribute__( (regparm(3) ) ) * __switch_to(task_pcb_t * curr, task_pcb_t * next) {
	// printk_debug("__switch_to\n");
	task_context_t * curr_context = curr->context;
	task_context_t * next_context = next->context;
	// printk_debug("curr_context: 0x%08X\t", curr_context);
	// printk_debug("next_context: 0x%08X\t", next_context);

	tss_set_gate(SEG_TSS, KERNEL_DS, next_context->esp);
	tss_load();

	__asm__ volatile ("mov %%fs,%0" : "=m" (curr->pt_regs->fs) );
	__asm__ volatile ("mov %%gs,%0" : "=m" (curr->pt_regs->gs) );
	// print_stack(15);
	// print_curr(curr_context);
	// print_next(next_context);
	// printk_debug("__switch_to end.\n");

	return curr;
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
