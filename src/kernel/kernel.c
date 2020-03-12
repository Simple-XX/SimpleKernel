
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// kernel.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "cpu.hpp"
#include "debug.h"
#include "assert.h"
#include "include/kernel.h"
#include "../drv/clock/include/clock.h"
#include "../drv/keyboard/include/keyboard.h"
#include "../test/include/test.h"
#include "../ds_alg/include/linkedlist.h"

static int32_t thread(void * arg) {
	for(int i = 0 ; i < 1009 ; i++) {
		printk("2");
	}
	asm ("hlt");
	return 1;
}

static int thread2(void * arg) {
	for(int i = 0 ; i < 100 ; i++) {
		printk("3");
	}
	return 3;
}

static int thread3(void * arg) {
	for(int i = 0 ; i < 100 ; i++) {
		printk("4");
	}
	return 4;
}

// 内核入口
void kernel_main(ptr_t magic, ptr_t addr) {
	cpu_cli();
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
	// 调试模块初始化
	debug_init(magic, addr);
	// 物理内存初始化
	pmm_init();
	// 虚拟内存初始化
	vmm_init();
	// 堆初始化
	heap_init();
	// 任务初始化
	task_init();
	// 调度初始化
	sched_init();

	// showinfo();
	// test();

	// kernel_thread(thread, NULL, 0);
	// kernel_thread(thread2, NULL, 0);
	// kernel_thread(thread3, NULL, 0);
	cpu_sti();

	for(int i = 0 ; i < 100 ; i++) {
		printk("8");
	}

	// 永远不会执行到这里
	assert(0, "Never to be seen.\n");
	cpu_hlt();
	return;
}

#ifdef __cplusplus
}
#endif
