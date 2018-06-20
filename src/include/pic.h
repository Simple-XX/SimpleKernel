
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// pic.h for MRNIU/SimpleKernel.




#ifndef _PIC_H_
#define _PIC_H_


#define PIC1_CMD                    0x20
#define PIC1_DATA                   0x21
#define PIC2_CMD                    0xA0
#define PIC2_DATA                   0xA1
#define PIC_READ_IRR                0x0a    /* OCW3 irq ready next CMD read */
#define PIC_READ_ISR                0x0b    /* OCW3 irq service next CMD read */

// Master - command: 0x20, data: 0x21
// Slave - command: 0xA0, data: 0xA1

void pic_init(){
  //PIC configuration:
  //设置主8259A和从8259A
  outb(0x20,0x11);
  outb(0xa0,0x11);
  //设置IRQ0-IRQ7的中断向量为0x20-0x27
  outb(0x21,0x20);
  //设置IRQ8-IRQ15的中断向量为0x28-0x2f
  outb(0xa1,0x28);
  //使从片PIC2连接到主片上
  outb(0x21,0x04);
  outb(0xa1,0x02);
  //打开8086模式
  outb(0x21,0x01);
  outb(0xa1,0x01);
  //关闭IRQ0-IRQ7的0x20-0x27中断
  outb(0x21,0xff);
  //关闭IRQ8-IRQ15的0x28-0x2f中断
  outb(0xa1,0xff);


 outb(0x20, 0x11);
 outb(0xA0, 0x11);

 outb(0x21, 0x20);
 outb(0xA1, 0x28);
 outb(0x21, 0x04);
 outb(0xA1, 0x02);
 outb(0x21, 0x01);
 outb(0xA1, 0x01);
 outb(0x21, 0x0);
 outb(0xA1, 0x0);
}


#endif
