
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// mouse.c for MRNIU/SimpleKernel.
#include "drv/mouse.h"
#include "drv/keyboard.h"
#include "port.hpp"


struct FIFO32 *mousefifo;
int mousedata0;

void mouse_callback(pt_regs_t * regs){
	uint32_t data;
	data = inb(MOU_DATA);
	return;
}

void enable_mouse(mouse_desc_t *mdec){
	/* 鼠标有效 */
	wait_to_write();
	outb(MOU_CMD, KEYCMD_SENDTO_MOUSE);
  wait_to_write();
	outb(MOU_DATA, MOUSECMD_ENABLE);
	/* 顺利的话，ACK(0xfa)会被发送*/
	mdec->phase = 0; /* 等待鼠标的0xfa的阶段*/
  return;
}

int mouse_decode(mouse_desc_t *mdec, uint8_t dat)
{
	if (mdec->phase == 0) {
		/* 等待鼠标的0xfa的阶段 */
		if (dat == 0xfa) {
			mdec->phase = 1;
		}
		return 0;
	}
	if (mdec->phase == 1) {
		/* 等待鼠标第一字节的阶段 */
		mdec->buf[0] = dat;
		mdec->phase = 2;
		return 0;
	}
	if (mdec->phase == 2) {
		/* 等待鼠标第二字节的阶段 */
		mdec->buf[1] = dat;
		mdec->phase = 3;
		return 0;
	}
	if (mdec->phase == 3) {
		/* 等待鼠标第二字节的阶段 */
		mdec->buf[2] = dat;
		mdec->phase = 1;
		mdec->btn = mdec->buf[0] & 0x07;
		mdec->x = mdec->buf[1];
		mdec->y = mdec->buf[2];
		if ((mdec->buf[0] & 0x10) != 0) {
			mdec->x |= 0xffffff00;
		}
		if ((mdec->buf[0] & 0x20) != 0) {
			mdec->y |= 0xffffff00;
		}
		mdec->y = - mdec->y; /* 鼠标的y方向与画面符号相反 */
		return 1;
	}
	/* 应该不可能到这里来 */
	return -1;
}

void init_mouse(void){
  enable_mouse();
  register_interrupt_handler(IRQ12, &mouse_decode);
}
