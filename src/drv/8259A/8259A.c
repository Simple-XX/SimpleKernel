
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// 8259A.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "include/8259A.h"
#include "sync.hpp"

void init_interrupt_chip(void) {
	bool intr_flag = false;
	local_intr_store(intr_flag);
	{
		// 重新映射 IRQ 表
		// 两片级联的 Intel 8259A 芯片
		// 主片端口 0x20 0x21
		// 从片端口 0xA0 0xA1

		// 初始化主片、从片
		// 0001 0001
		outb(IO_PIC1, 0x11);
		outb(IO_PIC1C, IRQ0);  // 设置主片 IRQ 从 0x20(32) 号中断开始
		outb(IO_PIC1C, 0x04);  // 设置主片 IR2 引脚连接从片
		outb(IO_PIC1C, 0x01);  // 设置主片按照 8086 的方式工作

		outb(IO_PIC2, 0x11);
		outb(IO_PIC2C, IRQ8);  // 设置从片 IRQ 从 0x28(40) 号中断开始
		outb(IO_PIC2C, 0x02);  // 告诉从片输出引脚和主片 IR2 号相连
		outb(IO_PIC2C, 0x01);  // 设置从片按照 8086 的方式工作

		// 默认关闭所有中断
		outb(IO_PIC1C, 0xFF);
		outb(IO_PIC2C, 0xFF);
	}
	local_intr_restore(intr_flag);
	return;
}

void clear_interrupt_chip(uint32_t intr_no) {
	bool intr_flag = false;
	local_intr_store(intr_flag);
	{
		// 发送中断结束信号给 PICs
		// 按照我们的设置，从 32 号中断起为用户自定义中断
		// 因为单片的 Intel 8259A 芯片只能处理 8 级中断
		// 故大于等于 40 的中断号是由从片处理的
		if(intr_no >= IRQ8) {
			// 发送重设信号给从片
			outb(IO_PIC2, PIC_EOI);
		}
		// 发送重设信号给主片
		outb(IO_PIC1, PIC_EOI);
	}
	local_intr_restore(intr_flag);
	return;
}

#ifdef __cplusplus
}
#endif
