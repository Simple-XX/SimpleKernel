
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// kernel.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "debug.h"
#include "include/kernel.h"
#include "../drv/clock/include/clock.h"
#include "../drv/keyboard/include/keyboard.h"
#include "../test/include/test.h"
#include "../ds_alg/include/linkedlist.h"

int flag = 0;

int thread(void * arg) {
	while(1) {
		if(flag == 1) {
			printk_test("B");
			flag = 0;
		}
	}

	return 0;
}

extern task_pcb_t kernel_task;
extern ListEntry * task_list;

// 内核入口
void kernel_main(ptr_t magic, ptr_t addr) {
	// 控制台初始化
	console_init();
	// 从 multiboot 获得系统初始信息
	multiboot2_init(magic, addr);
	// GDT、IDT 初始化
	arch_init();
	// 时钟初始化
	clock_init();
	// 键盘初始化
	keyboard_init();
	debug_init(magic, addr);
	// 物理内存初始化
	pmm_init();
	// 虚拟内存初始化
	vmm_init();
	// 堆初始化
	heap_init();
	// 任务初始化
	task_init();

	// showinfo();
	// test();

	// kernel_thread(thread, NULL, 0);
	// task_pcb_t * curr = get_current_task();
	// printk_debug("addr: 0x%08X\n", curr);
	// printk_debug("name: %s\n", curr->name);
	// struct task_struct * next = get_current_task();
	// switch_to(&(curr->context), &(current->context) );

	//
	// while(1) {
	// 	if(flag == 0) {
	// 		printk_test("A");
	// 		flag = 1;
	// 	}
	// }


	while(1);

	return;
}

#ifdef __cplusplus
}
#endif
