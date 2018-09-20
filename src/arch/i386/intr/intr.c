
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
// Based on http://wiki.0xffffff.org/posts/hurlex-kernel.html
// intr.c for MRNIU/SimpleKernel.


#include "intr.h"
#include "port.hpp"
#include "stdio.h"
#include "pic.hpp"
#include "cpu.hpp"


// 中断描述符表
static idt_entry_t idt_entries[INTERRUPT_MAX] __attribute__ ((aligned(16)));

static idt_ptr_t idt_ptr; // IDTR

// 设置中断描述符
static void idt_set_gate(uint8_t num, uint32_t base, uint16_t target, uint8_t flags){
  idt_entries[num].base_low = (base & 0xFFFF);
  idt_entries[num].base_high = (base >> 16) & 0xFFFF;
  idt_entries[num].selector = target;
  idt_entries[num].zero = 0;
  idt_entries[num].flags = flags;
  // 0x8E: DPL=0
  // 0xEF: DPL=3
}


// 中断处理函数指针数组
static interrupt_handler_t interrupt_handlers[INTERRUPT_MAX] __attribute__ ((aligned(4)));

static const char *intrname(uint32_t intrno){
  static const char *const intrnames[] = {
    "Divide error",
    "Debug",
    "Non-Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "BOUND Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection",
    "Page Fault",
    "(unknown trap)",
    "x87 FPU Floating-Point Error",
    "Alignment Check",
    "Machine-Check",
    "SIMD Floating-Point Exception"
  };
  if(intrno < sizeof(intrnames)/sizeof(const char *const)){
    return intrnames[intrno];
  }
  return "(unknown trap)";
}

// 调用中断处理函数
void isr_handler(pt_regs_t *regs){
  if (interrupt_handlers[regs->int_no]) {
    interrupt_handlers[regs->int_no](regs);
  }
  else {
    printk("Unhandled interrupt: %d %s\n", regs->int_no, intrname(regs->int_no));
    cpu_hlt();
  }
}

// 注册一个中断处理函数
void register_interrupt_handler(uint8_t n, interrupt_handler_t h){
  interrupt_handlers[n] = h;
}

// IRQ 处理函数
void irq_handler(pt_regs_t *regs){
	clear_interrupt_chip(regs->int_no);  // 重设PIC芯片
  if (interrupt_handlers[regs->int_no]) {
    interrupt_handlers[regs->int_no](regs);
  }
}


// 中断处理函数指针数组
static isr_irq_func_t isr_irq_func[INTERRUPT_MAX] = {
  [0]  = &isr0,  [1]  = &isr1,  [2]  = &isr2,  [3]  = &isr3,
  [4]  = &isr4,  [5]  = &isr5,  [6]  = &isr6,  [7]  = &isr7,
  [8]  = &isr8,  [9]  = &isr9,  [10] = &isr10, [11] = &isr11,
  [12] = &isr12, [13] = &isr13, [14] = &isr14, [15] = &isr15,
  [16] = &isr16, [17] = &isr17, [18] = &isr18, [19] = &isr19,
  [20] = &isr20, [21] = &isr21, [22] = &isr22, [23] = &isr23,
  [24] = &isr24, [25] = &isr25, [26] = &isr26, [27] = &isr27,
  [28] = &isr28, [29] = &isr29, [30] = &isr30, [31] = &isr31,

  [32] = &irq0,  [33] = &irq1,  [34] = &irq2,  [35] = &irq3,
  [36] = &irq4,  [37] = &irq5,  [38] = &irq6,  [39] = &irq7,
  [40] = &irq8,  [41] = &irq9,  [42] = &irq10, [43] = &irq11,
  [44] = &irq12, [45] = &irq13, [46] = &irq14, [47] = &irq15,
};

// idt 初始化
void idt_init(void){
  init_interrupt_chip();
  idt_ptr.limit = sizeof(idt_entry_t) * INTERRUPT_MAX -1;
  idt_ptr.base = (uint32_t)&idt_entries;

  // 0-32:  用于 CPU 的中断处理
  // GD_KTEXT: 内核代码段
  // 0x8E: 10001110: DPL=0s
  // 0x08: 0000 1000

  for(uint32_t i=0; i<48; ++i){
    idt_set_gate(i, (uint32_t)isr_irq_func[i], 0x08, 0x8E);
  }
  // 128 (0x80) 将来用于实现系统调用
  // 0xEF: 1110 1111, DPL=3
  idt_set_gate(INT_DEBUG, (uint32_t)isr_irq_func[INT_DEBUG], 0x08, 0xEF);
  idt_set_gate(INT_OVERFLOW, (uint32_t)isr_irq_func[INT_OVERFLOW], 0x08, 0xEF);
  idt_set_gate(INT_BOUND, (uint32_t)isr_irq_func[INT_BOUND], 0x08, 0xEF);
  idt_set_gate(128, (uint32_t)isr128, 0x08, 0xEF);

  idt_load((uint32_t)&idt_ptr);

  register_interrupt_handler(INT_DIVIDE_ERROR, &divide_error);
  register_interrupt_handler(INT_DEBUG, &debug);
  register_interrupt_handler(INT_NMI, &nmi);
  register_interrupt_handler(INT_BREAKPOINT, &breakpoint);
  register_interrupt_handler(INT_OVERFLOW, &overflow);
  register_interrupt_handler(INT_BOUND, &bound);
  register_interrupt_handler(INT_INVALID_OPCODE, &invalid_opcode);
  register_interrupt_handler(INT_DEVICE_NOT_AVAIL, &device_not_available);
  register_interrupt_handler(INT_DOUBLE_FAULT, &double_fault);
  register_interrupt_handler(INT_COPROCESSOR, &coprocessor_error);
  register_interrupt_handler(INT_INVALID_TSS, &invalid_TSS);
  register_interrupt_handler(INT_SEGMENT, &segment_not_present);
  register_interrupt_handler(INT_STACK_FAULT, &stack_segment);
  register_interrupt_handler(INT_GENERAL_PROTECT, &general_protection);
  register_interrupt_handler(INT_PAGE_FAULT, &page_fault);
}

static void die(char * str, uint32_t  oesp, uint32_t int_no){
  uint32_t * old_esp = (uint32_t *)oesp;
  printk_color(red, "%s\t: %d\n\r", str, int_no);
  printk_color(light_red, "Unuseable.\n");
  printk_color(red, "EIP:\t%08x:%p\nEFLAGS:\t%08x\nESP:\t%08x:%p\n",
          old_esp[1], read_eflags(), read_eflags(), old_esp[4], old_esp[3]);
          //old_esp[0]
  cpu_hlt();
}


void divide_error(pt_regs_t * regs){
  die("Divide Error.", regs->old_esp, regs->int_no);
}

void debug(pt_regs_t * regs){
  uint32_t tr;
  uint32_t * old_esp = (uint32_t *)regs->old_esp;

  // 取任务寄存器值->tr
	asm volatile("str %%ax"
              :"=a"(tr)
              :"0"(0));
  printk_color(light_red, "Unuseable.\n");

	printk_color(red, "eax\t\tebx\t\tecx\t\tedx\n\r%8X\t%8X\t%8X\t%8X\n\r",
		regs->eax, regs->ebx, regs->ecx, regs->edx);
	printk_color(red, "esi\t\tedi\t\tebp\t\tesp\n\r%8X\t%8X\t%8X\t%8X\n\r",
		           regs->esi, regs->edi, regs->ebp, (uint32_t) regs->user_esp);
	printk_color(red, "\n\rds\tes\tfs\tgs\n\r%4x\t%4x\t%4x\t%4x\n\r",
		           regs->ds, regs->es, regs->fs, regs->gs);
	printk_color(red, "EIP: %8X   EFLAGS: %d  CS: %4X\n\r",
               //old_esp[0], old_esp[1], old_esp[2]);
               old_esp[0], read_eflags(), old_esp[2]);
}

void nmi(pt_regs_t * regs){
  die("NMI.", regs->old_esp, regs->int_no);
}

void breakpoint(pt_regs_t * regs){
  die("Breakpoint.", regs->old_esp, regs->int_no);
}

void overflow(pt_regs_t * regs){
  die("Overflow.", regs->old_esp, regs->int_no);
}

void bound(pt_regs_t * regs){
  die("Bound.", regs->old_esp, regs->int_no);
}

void invalid_opcode(pt_regs_t * regs){
	die("Invalid Opcode.", regs->old_esp, regs->int_no);
}

void device_not_available(pt_regs_t * regs){
	die("Device Not Available.", regs->old_esp, regs->int_no);
}

void double_fault(pt_regs_t * regs){
  die("Double Fault.", regs->old_esp, regs->int_no);
}

void coprocessor_error(pt_regs_t * regs){
	die("Coprocessor Error.", regs->old_esp, regs->int_no);
}

void invalid_TSS(pt_regs_t * regs){
	die("Invalid TSS.", regs->old_esp, regs->int_no);
}

void segment_not_present(pt_regs_t * regs){
	die("Segment Not Present.", regs->old_esp, regs->int_no);
}

void stack_segment(pt_regs_t * regs){
	die("Stack Segment.", regs->old_esp, regs->int_no);
}

void general_protection(pt_regs_t * regs){
  die("General Protection.", regs->old_esp, regs->int_no);
}

void page_fault(pt_regs_t * regs){
  die("Page Fault.", regs->old_esp, regs->int_no);
}
