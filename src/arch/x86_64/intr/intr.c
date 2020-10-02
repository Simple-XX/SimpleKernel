
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel). Based on
// http://wiki.0xffffff.org/posts/hurlex-kernel.html intr.c for
// Simple-XX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "intr.h"
#include "8259A.h"
// #include "port.hpp"
// #include "cpu.hpp"

// 中断描述符表
static idt_entry_t idt_entries[INTERRUPT_MAX] __attribute__((aligned(16)));

static idt_ptr_t idt_ptr; // IDTR

// 设置中断描述符
static void idt_set_gate(uint8_t num, uint32_t base, uint16_t target,
                         uint8_t flags) {
    idt_entries[num].base_low  = (base & 0xFFFF);
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;
    idt_entries[num].selector  = target;
    idt_entries[num].zero      = 0;
    idt_entries[num].flags     = flags;
    // 0x8E: DPL=0
    // 0xEF: DPL=3
}

// 中断处理函数指针数组
static interrupt_handler_t interrupt_handlers[INTERRUPT_MAX]
    __attribute__((aligned(4)));

static const char *intrname(uint32_t intrno) {
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

// 调用中断处理函数
void isr_handler(pt_regs_t *regs) {
    if (interrupt_handlers[regs->int_no]) {
        interrupt_handlers[regs->int_no](regs);
    }
    else {
        printk("Unhandled interrupt: %d %s\n", regs->int_no,
               intrname(regs->int_no));
        cpu_hlt();
    }
}

// 注册一个中断处理函数
void register_interrupt_handler(uint8_t n, interrupt_handler_t h) {
    interrupt_handlers[n] = h;
}

// IRQ 处理函数
void irq_handler(pt_regs_t *regs) {
    clear_interrupt_chip(regs->int_no); // 重设PIC芯片
    if (interrupt_handlers[regs->int_no]) {
        interrupt_handlers[regs->int_no](regs);
    }
}

// 中断处理函数指针数组
static isr_irq_func_t isr_irq_func[INTERRUPT_MAX] = {
    [0] = &isr0,   [1] = &isr1,   [2] = &isr2,   [3] = &isr3,   [4] = &isr4,
    [5] = &isr5,   [6] = &isr6,   [7] = &isr7,   [8] = &isr8,   [9] = &isr9,
    [10] = &isr10, [11] = &isr11, [12] = &isr12, [13] = &isr13, [14] = &isr14,
    [15] = &isr15, [16] = &isr16, [17] = &isr17, [18] = &isr18, [19] = &isr19,
    [20] = &isr20, [21] = &isr21, [22] = &isr22, [23] = &isr23, [24] = &isr24,
    [25] = &isr25, [26] = &isr26, [27] = &isr27, [28] = &isr28, [29] = &isr29,
    [30] = &isr30, [31] = &isr31,

    [32] = &irq0,  [33] = &irq1,  [34] = &irq2,  [35] = &irq3,  [36] = &irq4,
    [37] = &irq5,  [38] = &irq6,  [39] = &irq7,  [40] = &irq8,  [41] = &irq9,
    [42] = &irq10, [43] = &irq11, [44] = &irq12, [45] = &irq13, [46] = &irq14,
    [47] = &irq15,
};

// idt 初始化
void idt_init(void) {
    init_interrupt_chip();
    idt_ptr.limit = sizeof(idt_entry_t) * INTERRUPT_MAX - 1;
    idt_ptr.base  = (uint32_t)&idt_entries;

    // 0-32:  用于 CPU 的中断处理
    // GD_KTEXT: 内核代码段
    // 0x8E: 10001110: DPL=0s
    // 0x08: 0000 1000

    for (uint32_t i = 0; i < 48; ++i) {
        idt_set_gate(i, (uint32_t)isr_irq_func[i], 0x08, 0x8E);
    }
    // 128 (0x80) 将来用于实现系统调用
    // 0xEF: 1110 1111, DPL=3
    idt_set_gate(INT_DEBUG, (uint32_t)isr_irq_func[INT_DEBUG], 0x08, 0xEF);
    idt_set_gate(INT_OVERFLOW, (uint32_t)isr_irq_func[INT_OVERFLOW], 0x08,
                 0xEF);
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

    printk_info("intr_init\n");
}

static void die(char *str, uint32_t oesp, uint32_t int_no) {
    // uint32_t * old_esp = (uint32_t *)oesp;
    pt_regs_t *old_esp = (pt_regs_t *)oesp;
    printk_color(red, "%s\t: %d\n\r", str, int_no);
    printk_color(light_red, "die_Unuseable.\n");
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
    printk_color(red, "gs: %08x\tfs: %08x\tes: %08x\tds: %08x\n", old_esp->gs,
                 old_esp->fs, old_esp->es, old_esp->ds);
    printk_color(red, "edi: %08x\tesi: %08x\tebp: %08x\told_esp: %08x\n",
                 old_esp->edi, old_esp->esi, old_esp->ebp, old_esp->old_esp);
    printk_color(red, "ebx: %08x\tedx: %08x\tecx: %08x\teax: %08x\n",
                 old_esp->ebx, old_esp->edx, old_esp->ecx, old_esp->eax);
    printk_color(red, "int_no: %08X\terr_code: %08X\teip: %08x\tcs: %08x\n",
                 old_esp->int_no, old_esp->err_code, old_esp->eip, old_esp->cs);
    printk_color(red, "eflags: %08x\tuser_esp: %08x\tss: %08x\n",
                 old_esp->eflags, old_esp->user_esp, old_esp->user_ss);
    printk_color(red, "addr: %08x, %08X\n", &old_esp->gs, &old_esp->user_ss);

    cpu_hlt();
}

void divide_error(pt_regs_t *regs) {
    die("Divide Error.", regs->old_esp, regs->int_no);
}

void debug(pt_regs_t *regs) {
    uint32_t  tr;
    uint32_t *old_esp = (uint32_t *)regs->old_esp;

    // 取任务寄存器值->tr
    __asm__ volatile("str %%ax" : "=a"(tr) : "0"(0));
    printk_color(light_red, "Unuseable.\n");

    printk_color(red, "eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                 regs->eax, regs->ebx, regs->ecx, regs->edx);
    printk_color(red, "esi 0x%08X\tedi 0x%08X\tebp 0x%08X\tesp 0x%08X\n",
                 regs->esi, regs->edi, regs->ebp, (uint32_t)regs->user_esp);
    printk_color(red, "ds 0x%08X\tes 0x%08X\tfs 0x%08X\tgs 0x%08X\n", regs->ds,
                 regs->es, regs->fs, regs->gs);
    printk_color(red, "EIP: 0x%08X\tEFLAGS: 0x%08X\tCS: 0x%08X\n",
                 // old_esp[0], old_esp[1], old_esp[2]);
                 old_esp[0], read_eflags(), old_esp[2]);
    return;
}

void nmi(pt_regs_t *regs) {
    die("NMI.", regs->old_esp, regs->int_no);
}

void breakpoint(pt_regs_t *regs) {
    die("Breakpoint.", regs->old_esp, regs->int_no);
}

void overflow(pt_regs_t *regs) {
    die("Overflow.", regs->old_esp, regs->int_no);
}

void bound(pt_regs_t *regs) {
    die("Bound.", regs->old_esp, regs->int_no);
}

void invalid_opcode(pt_regs_t *regs) {
    die("Invalid Opcode.", regs->old_esp, regs->int_no);
}

void device_not_available(pt_regs_t *regs) {
    die("Device Not Available.", regs->old_esp, regs->int_no);
}

void double_fault(pt_regs_t *regs) {
    die("Double Fault.", regs->old_esp, regs->int_no);
}

void coprocessor_error(pt_regs_t *regs) {
    die("Coprocessor Error.", regs->old_esp, regs->int_no);
}

void invalid_TSS(pt_regs_t *regs) {
    die("Invalid TSS.", regs->old_esp, regs->int_no);
}

void segment_not_present(pt_regs_t *regs) {
    die("Segment Not Present.", regs->old_esp, regs->int_no);
}

void stack_segment(pt_regs_t *regs) {
    die("Stack Segment.", regs->old_esp, regs->int_no);
}

void general_protection(pt_regs_t *regs) {
    die("General Protection.", regs->old_esp, regs->int_no);
}

void page_fault(pt_regs_t *regs) {
#ifdef __x86_64__
    uint64_t cr2;
    asm volatile("movq %%cr2,%0" : "=r"(cr2));
#else
    uint32_t cr2;
    asm volatile("mov %%cr2,%0" : "=r"(cr2));
#endif
    printk("Page fault at 0x%08X, virtual faulting address 0x%08X\n", regs->eip,
           cr2);
    printk_err("Error code: 0x%08X\n", regs->err_code);

    // bit 0 为 0 指页面不存在内存里
    if (!(regs->err_code & 0x1))
        printk_color(red, "Because the page wasn't present.\n");
    // bit 1 为 0 表示读错误，为 1 为写错误
    if (regs->err_code & 0x2)
        printk_err("Write error.\n");
    else
        printk_err("Read error.\n");
    // bit 2 为 1 表示在用户模式打断的，为 0 是在内核模式打断的
    if (regs->err_code & 0x4)
        printk_err("In user mode.\n");
    else
        printk_err("In kernel mode.\n");
    // bit 3 为 1 表示错误是由保留位覆盖造成的
    if (regs->err_code & 0x8)
        printk_err("Reserved bits being overwritten.\n");
    // bit 4 为 1 表示错误发生在取指令的时候
    if (regs->err_code & 0x10)
        printk_err("The fault occurred during an instruction fetch.\n");
    while (1)
        ;
}

void enable_irq(uint32_t irq_no) {
    uint8_t mask = 0;
    // printk_color(green, "enable_irq mask: %X", mask);
    if (irq_no >= IRQ8) {
        mask = ((inb(IO_PIC2C)) & (~(1 << (irq_no % 8))));
        outb(IO_PIC2C, mask);
    }
    else {
        mask = ((inb(IO_PIC1C)) & (~(1 << (irq_no % 8))));
        outb(IO_PIC1C, mask);
    }
}

void disable_irq(uint32_t irq_no) {
    uint8_t mask = 0;
    // printk_color(green, "disable_irq mask: %X", mask);
    if (irq_no >= IRQ8) {
        mask = ((inb(IO_PIC2C)) | (1 << (irq_no % 8)));
        outb(IO_PIC2C, mask);
    }
    else {
        mask = ((inb(IO_PIC1C)) | (1 << (irq_no % 8)));
        outb(IO_PIC1C, mask);
    }
}

#ifdef __cplusplus
}
#endif
