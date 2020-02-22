
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// sched.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "assert.h"
#include "cpu.hpp"
#include "include/linkedlist.h"
#include "intr/include/intr.h"
#include "sched/sched.h"
#include "debug.h"

int c = 0;

void clock_handler(pt_regs_t * regs UNUSED) {
	// if(c++ > 1) {
	// cpu_hlt();
	// printk_debug(" \
	// 	    gs: 0x%08X\tfs: %08X\tes: %08X\tds: %08X\t \
	// 	    edi: %08X\tesi: %08X\tebp: %08X\told_esp: %08X\t \
	// 	    ebx: %08X\tedx: %08X\tecx: %08X\teax: %08X\t \
	// 	    int_no: %08X\terr_code: %08X\teip: %08X\tcs: %08X\t \
	// 	    eflags: %08X\tuser_esp: %08X\tss: %08X\n \
	// 	    ",
	//     regs->gs, regs->fs, regs->es, regs->ds,
	//     regs->edi, regs->esi, regs->ebp, regs->old_esp,
	//     regs->ebx, regs->edx, regs->ecx, regs->eax,
	//     regs->int_no, regs->err_code, regs->eip, regs->cs,
	//     regs->eflags, regs->user_esp, regs->user_ss);

	// }
	schedule();
	// }
	return;
}

void sched_init() {
	cpu_cli();
	// 注册时间相关的处理函数
	register_interrupt_handler(IRQ0, &clock_handler);
	enable_irq(IRQ0);
	printk_info("sched init\n");
	return;
}

void sched_switch(task_context_t * curr UNUSED, task_context_t * next UNUSED) {
	return;
}

// 比较方法
static int vs_med(void * v1, void * v2) {
	return v1 == v2;
}

void schedule() {
	cpu_cli();
	// 首先从链表中找到正在执行的进程，结果不为空
	ListEntry * tmp = list_find_data(runnable_list, vs_med, curr_task);
	assert( (tmp != NULL), "Error at sched.c, tmp == NULL!\n");
	task_pcb_t * next =
	    ( ( (task_pcb_t *)(list_data(list_next(tmp) ) ) ) == NULL) ?
	    ( (task_pcb_t *)(list_nth_data(runnable_list, 0) ) )
	    : ( (task_pcb_t *)(list_data(list_next(tmp) ) ) );
	if( (curr_task->pid != next->pid) ) {
		task_pcb_t * prev = curr_task;
		curr_task = next;
		printk_debug("switch_to: \n");
		// printk_debug("prev->context: 0x%08X\t", prev->context);
		// printk_debug("prev->pid: 0x%08X\t", prev->pid);
		// printk_debug("prev->name: %s\t", prev->name);
		// printk_debug("prev->eip 0x%08X\t", prev->context->eip);
		// printk_debug("prev->esp 0x%08X\t", prev->context->esp);
		// printk_debug("prev->ebp 0x%08X\t", prev->context->ebp);
		// printk_debug("prev->ebx 0x%08X\t", prev->context->ebx);
		// printk_debug("prev->ecx 0x%08X\t", prev->context->ecx);
		// printk_debug("prev->edx 0x%08X\t", prev->context->edx);
		// printk_debug("prev->esi 0x%08X\t", prev->context->esi);
		// printk_debug("prev->edi 0x%08X\n", prev->context->edi);
		// printk_debug("next->context: 0x%08X\t", next->context);
		// printk_debug("next->pid: 0x%08X\t", next->pid);
		// printk_debug("next->name: %s\t", next->name);
		// printk_debug("next->eip 0x%08X\t", next->context->eip);
		// printk_debug("next->esp 0x%08X\t", next->context->esp);
		// printk_debug("next->ebp 0x%08X\t", next->context->ebp);
		// printk_debug("next->ebx 0x%08X\t", next->context->ebx);
		// printk_debug("next->ecx 0x%08X\t", next->context->ecx);
		// printk_debug("next->edx 0x%08X\t", next->context->edx);
		// printk_debug("next->esi 0x%08X\t", next->context->esi);
		// printk_debug("next->edi 0x%08X\n", next->context->edi);
		print_stack(15);
		printk_debug("switch_to-----\n");
		switch_to( (prev->context), (curr_task->context) );
		printk_debug("switch_to END.\n");
		asm ("hlt");
	}
	cpu_sti();
	return;
}

#ifdef __cplusplus
}
#endif
