
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// sched.c for MRNIU/SimpleKernel.


#ifdef __cplusplus
extern "C" {
#endif

#include "sched/sched.h"

// 可调度进程链表
task_pcb_t * running_proc_head = NULL;

// 等待进程链表
task_pcb_t * wait_proc_head = NULL;

// 当前运行的任务
task_pcb_t * current = NULL;

void sched_init() {
	// 为当前执行流创建信息结构体 该结构位于当前执行流的栈最低端
	current = (task_pcb_t *)(kernel_stack_top - STACK_SIZE);

	current->state = TASK_RUNNABLE;
	current->pid = curr_pid++;
	current->stack = (ptr_t)current;   // 该成员指向栈低地址
	current->mm = NULL;         // 内核线程不需要该成员

	// 单向循环链表
	current->next = (task_pcb_t *)current;

	running_proc_head = current;
}

void schedule() {
	if(current) {
		change_task_to(current->next);
	}
}

void change_task_to(task_pcb_t * next) {
	if(current != next) {
		task_pcb_t * prev = current;
		current = next;
		switch_to(&(prev->context), &(current->context) );
	}
}

#ifdef __cplusplus
}
#endif
