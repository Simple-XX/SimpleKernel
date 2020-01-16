
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// task.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "string.h"
#include "assert.h"
#include "task/task.h"
#include "sched/sched.h"

// 全局 pid 值
pid_t curr_pid = 0;

void task_init(void) {
	return;
}

// 线程创建
pid_t kfork(int (* fn)(void *), void * arg)
{
	task_pcb_t * new_task = (task_pcb_t *)kmalloc(STACK_SIZE);
	assert(new_task != NULL, "kern_thread: kmalloc error");

	// 将栈低端结构信息初始化为 0
	bzero(new_task, sizeof(task_pcb_t) );

	new_task->state = TASK_RUNNABLE;
	new_task->stack = (ptr_t)current;
	new_task->pid = curr_pid++;
	new_task->mm = NULL;

	uint32_t * stack_top = (uint32_t *)( (uint32_t)new_task + STACK_SIZE);

	*(--stack_top) = (uint32_t)arg;
	*(--stack_top) = (uint32_t)kthread_exit;
	*(--stack_top) = (uint32_t)fn;

	new_task->context.esp = (uint32_t)new_task + STACK_SIZE - sizeof(uint32_t) * 3;

	// 设置新任务的标志寄存器未屏蔽中断，很重要
	// new_task->context.eflags = 0x200;
	new_task->next = running_proc_head;

	// 找到当前进任务队列，插入到末尾
	task_pcb_t * tail = running_proc_head;
	assert(tail != NULL, "Must init sched!");

	while(tail->next != running_proc_head) {
		tail = tail->next;
	}
	tail->next = new_task;

	return new_task->pid;
}

void kexit()
{
	register uint32_t res __asm__ ("eax");

	printk("Thread exited with value %d\n", res);

	while(1);
}

#ifdef __cplusplus
}
#endif
