
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on http://wiki.0xffffff.org/posts/hurlex-kernel.html
// intr.cpp for Simple-XX/SimpleKernel.

#include "intr.h"
#include "console.h"
#include "cpu.hpp"

void die(const char *str, uint32_t oesp, uint32_t int_no) {
    // uint32_t * old_esp = (uint32_t *)oesp;
    pt_regs_t *old_esp = (pt_regs_t *)oesp;
    consolek.printk("%s\t: %d\n\r", str, int_no);
    consolek.printk("die_Unuseable.\n");
    // cs::EIP
    // oss:oesp
    // printk_color(red, "EIP:\t%08x:%p\nEFLAGS:\t%08x\nESP:\t%08x:%p\n",
    //              old_esp[1], read_eflags(), read_eflags(), old_esp[4],
    //              old_esp[3]);
    // printk_color(red, "EIP:\t%08x:%p\nEFLAGS:\t%08x\nESP:\t%08x:%p\n",
    //              &old_esp[1], &old_esp[0], &old_esp[2], &old_esp[4],
    //              &old_esp[3]);
    // printk_color(red, "EIP:\t%08x:%08X\nEFLAGS:\t%08x\nESP:\t%08x:%08X\n",
    // old_esp->cs, old_esp->eip, old_esp->eflags, old_esp->ss,
    // old_esp->old_esp);
    consolek.printk("gs: %08x\tfs: %08x\tes: %08x\tds: %08x\n", old_esp->gs,
                    old_esp->fs, old_esp->es, old_esp->ds);
    consolek.printk("edi: %08x\tesi: %08x\tebp: %08x\told_esp: %08x\n",
                    old_esp->edi, old_esp->esi, old_esp->ebp, old_esp->old_esp);
    consolek.printk("ebx: %08x\tedx: %08x\tecx: %08x\teax: %08x\n",
                    old_esp->ebx, old_esp->edx, old_esp->ecx, old_esp->eax);
    consolek.printk("int_no: %08X\terr_code: %08X\teip: %08x\tcs: %08x\n",
                    old_esp->int_no, old_esp->err_code, old_esp->eip,
                    old_esp->cs);
    consolek.printk("eflags: %08x\tuser_esp: %08x\tss: %08x\n", old_esp->eflags,
                    old_esp->user_esp, old_esp->user_ss);
    consolek.printk("addr: %08x, %08X\n", &old_esp->gs, &old_esp->user_ss);

    cpu_hlt();
    return;
}

void irq_handler(pt_regs_t *regs) {
    // 重设PIC芯片
    INTR::clear_interrupt_chip(regs->int_no);
    if (INTR::interrupt_handlers[regs->int_no]) {
        INTR::interrupt_handlers[regs->int_no](regs);
    }
    return;
}

void isr_handler(pt_regs_t *regs) {
    if (INTR::interrupt_handlers[regs->int_no]) {
        INTR::interrupt_handlers[regs->int_no](regs);
    }
    else {
        consolek.printk("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                        "Unhandled interrupt: %d %s\n", regs->int_no,
                        INTR::intrname(regs->int_no));
        cpu_hlt();
    }
    return;
}

isr_irq_func_t      INTR::isr_irq_func[INTERRUPT_MAX];
interrupt_handler_t INTR::interrupt_handlers[INTERRUPT_MAX]
    __attribute__((aligned(4)));
idt_entry_t INTR::idt_entries[INTERRUPT_MAX] __attribute__((aligned(16)));
idt_ptr_t   INTR::idt_ptr;

INTR::INTR(void) {
    isr_irq_func[0]  = &isr0;
    isr_irq_func[1]  = &isr1;
    isr_irq_func[2]  = &isr2;
    isr_irq_func[3]  = &isr3;
    isr_irq_func[4]  = &isr4;
    isr_irq_func[5]  = &isr5;
    isr_irq_func[6]  = &isr6;
    isr_irq_func[7]  = &isr7;
    isr_irq_func[8]  = &isr8;
    isr_irq_func[9]  = &isr9;
    isr_irq_func[10] = &isr10;
    isr_irq_func[11] = &isr11;
    isr_irq_func[12] = &isr12;
    isr_irq_func[13] = &isr13;
    isr_irq_func[14] = &isr14;
    isr_irq_func[15] = &isr15;
    isr_irq_func[16] = &isr16;
    isr_irq_func[17] = &isr17;
    isr_irq_func[18] = &isr18;
    isr_irq_func[19] = &isr19;
    isr_irq_func[20] = &isr20;
    isr_irq_func[21] = &isr21;
    isr_irq_func[22] = &isr22;
    isr_irq_func[23] = &isr23;
    isr_irq_func[24] = &isr24;
    isr_irq_func[25] = &isr25;
    isr_irq_func[26] = &isr26;
    isr_irq_func[27] = &isr27;
    isr_irq_func[28] = &isr28;
    isr_irq_func[29] = &isr29;
    isr_irq_func[30] = &isr30;
    isr_irq_func[31] = &isr31;

    isr_irq_func[32] = &irq0;
    isr_irq_func[33] = &irq1;
    isr_irq_func[34] = &irq2;
    isr_irq_func[35] = &irq3;
    isr_irq_func[36] = &irq4;
    isr_irq_func[37] = &irq5;
    isr_irq_func[38] = &irq6;
    isr_irq_func[39] = &irq7;
    isr_irq_func[40] = &irq8;
    isr_irq_func[41] = &irq9;
    isr_irq_func[42] = &irq10;
    isr_irq_func[43] = &irq11;
    isr_irq_func[44] = &irq12;
    isr_irq_func[45] = &irq13;
    isr_irq_func[46] = &irq14;
    isr_irq_func[47] = &irq15;
    // idt 初始化
    init_interrupt_chip();
    idt_ptr.limit = sizeof(idt_entry_t) * INTERRUPT_MAX - 1;
    idt_ptr.base  = (uint32_t)&idt_entries;

    // 0-32:  用于 CPU 的中断处理
    // GD_KTEXT: 内核代码段
    // 0x8E: 10001110: DPL=0s
    // 0x08: 0000 1000

    for (uint32_t i = 0; i < 48; ++i) {
        set_idt(i, (uint32_t)isr_irq_func[i], 0x08, 0x8E);
    }
    // 128 (0x80) 将来用于实现系统调用
    // 0xEF: 1110 1111, DPL=3
    set_idt(INT_DEBUG, (uint32_t)isr_irq_func[INT_DEBUG], 0x08, 0xEF);
    set_idt(INT_OVERFLOW, (uint32_t)isr_irq_func[INT_OVERFLOW], 0x08, 0xEF);
    set_idt(INT_BOUND, (uint32_t)isr_irq_func[INT_BOUND], 0x08, 0xEF);
    set_idt(128, (uint32_t)isr128, 0x08, 0xEF);

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

    consolek.printk("intr_init\n");
    return;
}

INTR::~INTR(void) {
    return;
}

void INTR::divide_error(pt_regs_t *regs) {
    die("Divide Error.", regs->old_esp, regs->int_no);
    return;
}

void INTR::debug(pt_regs_t *regs) {
    uint32_t  tr;
    uint32_t *old_esp = (uint32_t *)regs->old_esp;

    // 取任务寄存器值->tr
    __asm__ volatile("str %%ax" : "=a"(tr) : "0"(0));
    consolek.printk("Unuseable.\n");

    consolek.printk("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                    regs->eax, regs->ebx, regs->ecx, regs->edx);
    consolek.printk("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                    "esi 0x%08X\tedi 0x%08X\tebp 0x%08X\tesp 0x%08X\n",
                    regs->esi, regs->edi, regs->ebp, (uint32_t)regs->user_esp);
    consolek.printk("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                    "ds 0x%08X\tes 0x%08X\tfs 0x%08X\tgs 0x%08X\n", regs->ds,
                    regs->es, regs->fs, regs->gs);
    consolek.printk("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                    "EIP: 0x%08X\tEFLAGS: 0x%08X\tCS: 0x%08X\n",
                    // old_esp[0], old_esp[1], old_esp[2]);
                    old_esp[0], read_eflags(), old_esp[2]);
    return;
}

void INTR::nmi(pt_regs_t *regs) {
    die("NMI.", regs->old_esp, regs->int_no);
    return;
}

void INTR::breakpoint(pt_regs_t *regs) {
    die("Breakpoint.", regs->old_esp, regs->int_no);
    return;
}

void INTR::overflow(pt_regs_t *regs) {
    die("Overflow.", regs->old_esp, regs->int_no);
    return;
}

void INTR::bound(pt_regs_t *regs) {
    die("Bound.", regs->old_esp, regs->int_no);
    return;
}

void INTR::invalid_opcode(pt_regs_t *regs) {
    die("Invalid Opcode.", regs->old_esp, regs->int_no);
    return;
}

void INTR::device_not_available(pt_regs_t *regs) {
    die("Device Not Available.", regs->old_esp, regs->int_no);
    return;
}

void INTR::double_fault(pt_regs_t *regs) {
    die("Double Fault.", regs->old_esp, regs->int_no);
    return;
}

void INTR::coprocessor_error(pt_regs_t *regs) {
    die("Coprocessor Error.", regs->old_esp, regs->int_no);
    return;
}

void INTR::invalid_TSS(pt_regs_t *regs) {
    die("Invalid TSS.", regs->old_esp, regs->int_no);
    return;
}

void INTR::segment_not_present(pt_regs_t *regs) {
    die("Segment Not Present.", regs->old_esp, regs->int_no);
    return;
}

void INTR::stack_segment(pt_regs_t *regs) {
    die("Stack Segment.", regs->old_esp, regs->int_no);
    return;
}

void INTR::general_protection(pt_regs_t *regs) {
    die("General Protection.", regs->old_esp, regs->int_no);
    return;
}

void INTR::page_fault(pt_regs_t *regs) {
#ifdef __x86_64__
    uint64_t cr2;
    asm volatile("movq %%cr2,%0" : "=r"(cr2));
#else
    uint32_t cr2;
    asm volatile("mov %%cr2,%0" : "=r"(cr2));
#endif
    consolek.printk("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                    "Page fault at 0x%08X, virtual faulting address 0x%08X\n",
                    regs->eip, cr2);
    consolek.printk("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                    "Error code: 0x%08X\n", regs->err_code);

    // bit 0 为 0 指页面不存在内存里
    if (!(regs->err_code & 0x1)) {
        consolek.printk("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                        "Because the page wasn't present.\n");
    }
    // bit 1 为 0 表示读错误，为 1 为写错误
    if (regs->err_code & 0x2) {
        consolek.printk("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                        "Write error.\n");
    }
    else {
        consolek.printk("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                        "Read error.\n");
    }
    // bit 2 为 1 表示在用户模式打断的，为 0 是在内核模式打断的
    if (regs->err_code & 0x4) {
        consolek.printk("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                        "In user mode.\n");
    }
    else {
        consolek.printk("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                        "In kernel mode.\n");
    }
    // bit 3 为 1 表示错误是由保留位覆盖造成的
    if (regs->err_code & 0x8) {
        consolek.printk("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                        "Reserved bits being overwritten.\n");
    }
    // bit 4 为 1 表示错误发生在取指令的时候
    if (regs->err_code & 0x10) {
        consolek.printk("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                        "The fault occurred during an instruction fetch.\n");
    }
    while (1) {
        ;
    }
    return;
}

void INTR::set_idt(uint8_t num, uint32_t base, uint16_t target, uint8_t flags) {
    idt_entries[num].base_low  = (base & 0xFFFF);
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;
    idt_entries[num].selector  = target;
    idt_entries[num].zero      = 0;
    idt_entries[num].flags     = flags;
    // 0x8E: DPL=0
    // 0xEF: DPL=3
    return;
}

const char *INTR::intrname(uint32_t intrno) {
    static const char *const intrnames[] = {"Divide error",
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
                                            "SIMD Floating-Point Exception"};
    if (intrno < sizeof(intrnames) / sizeof(const char *const)) {
        return intrnames[intrno];
    }
    return "(unknown trap)";
}

void INTR::register_interrupt_handler(uint8_t n, interrupt_handler_t h) {
    interrupt_handlers[n] = h;
    return;
}

void INTR::enable_irq(uint32_t irq_no) {
    uint8_t mask = 0;
    // printk_color(green, "enable_irq mask: %X", mask);
    if (irq_no >= IRQ8) {
        mask = ((PORT::inb(IO_PIC2C)) & (~(1 << (irq_no % 8))));
        PORT::outb(IO_PIC2C, mask);
    }
    else {
        mask = ((PORT::inb(IO_PIC1C)) & (~(1 << (irq_no % 8))));
        PORT::outb(IO_PIC1C, mask);
    }
    return;
}

void INTR::disable_irq(uint32_t irq_no) {
    uint8_t mask = 0;
    // printk_color(green, "disable_irq mask: %X", mask);
    if (irq_no >= IRQ8) {
        mask = ((PORT::inb(IO_PIC2C)) | (1 << (irq_no % 8)));
        PORT::outb(IO_PIC2C, mask);
    }
    else {
        mask = ((PORT::inb(IO_PIC1C)) | (1 << (irq_no % 8)));
        PORT::outb(IO_PIC1C, mask);
    }
    return;
}

INTR intrk;
