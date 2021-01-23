
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on http://wiki.0xffffff.org/posts/hurlex-kernel.html
// intr.cpp for Simple-XX/SimpleKernel.

#include "intr.h"
#include "cpu.hpp"
#include "io.h"

namespace INTR {

#ifdef __cplusplus
    extern "C" {
#endif

    // 声明中断处理函数 0 ~ 19 属于 CPU 的异常中断
    // ISR:中断服务程序(interrupt service routine)
    // 0 #DE 除 0 异常
    extern void isr0();
    // 1 #DB 调试异常
    extern void isr1();
    // 2 NMI
    extern void isr2();
    // 3 BP 断点异常
    extern void isr3();
    // 4 #OF 溢出
    extern void isr4();
    // 5 #BR 对数组的引用超出边界
    extern void isr5();
    // 6 #UD 无效或未定义的操作码
    extern void isr6();
    // 7 #NM 设备不可用(无数学协处理器)
    extern void isr7();
    // 8 #DF 双重故障(有错误代码)
    extern void isr8();
    // 9 协处理器跨段操作
    extern void isr9();
    // 10 #TS 无效TSS(有错误代码)
    extern void isr10();
    // 11 #NP 段不存在(有错误代码)
    extern void isr11();
    // 12 #SS 栈错误(有错误代码)
    extern void isr12();
    // 13 #GP 常规保护(有错误代码)
    extern void isr13();
    // 14 #PF 页故障(有错误代码)
    extern void isr14();
    // 15 CPU 保留
    extern void isr15();
    // 16 #MF 浮点处理单元错误
    extern void isr16();
    // 17 #AC 对齐检查
    extern void isr17();
    // 18 #MC 机器检查
    extern void isr18();
    // 19 #XM SIMD(单指令多数据)浮点异常
    extern void isr19();

    // 20 ~ 31 Intel 保留
    extern void isr20();
    extern void isr21();
    extern void isr22();
    extern void isr23();
    extern void isr24();
    extern void isr25();
    extern void isr26();
    extern void isr27();
    extern void isr28();
    extern void isr29();
    extern void isr30();
    extern void isr31();

    // 32 ~ 255 用户自定义异常
    extern void isr128(); // 0x80 用于实现系统调用

    // 声明 IRQ 函数
    // IRQ:中断请求(Interrupt Request)
    // 电脑系统计时器
    extern void irq0();
    // 键盘
    extern void irq1();
    // 与 IRQ9 相接，MPU-401 MD 使用
    extern void irq2();
    // 串口设备
    extern void irq3();
    // 串口设备
    extern void irq4();
    // 建议声卡使用
    extern void irq5();
    // 软驱传输控制使用
    extern void irq6();
    // 打印机传输控制使用
    extern void irq7();
    // 即时时钟
    extern void irq8();
    // 与 IRQ2 相接，可设定给其他硬件
    extern void irq9();
    // 建议网卡使用
    extern void irq10();
    // 建议 AGP 显卡使用
    extern void irq11();
    // 接 PS/2 鼠标，也可设定给其他硬件
    extern void irq12();
    // 协处理器使用
    extern void irq13();
    // IDE0 传输控制使用
    extern void irq14();
    // IDE1 传输控制使用
    extern void irq15();

    // 声明加载 IDTR 的函数
    extern void idt_load(uint32_t);

    // IRQ 处理函数
    void irq_handler(pt_regs_t *regs) {
        call_irq(regs->int_no, regs);
        return;
    }

    // ISR 处理函数
    void isr_handler(pt_regs_t *regs) {
        call_isr(regs->int_no, regs);
        return;
    }

#ifdef __cplusplus
    }
#endif

    // 中断处理函数指针数组
    static isr_irq_func_t      isr_irq_func[INTERRUPT_MAX];
    static interrupt_handler_t interrupt_handlers[INTERRUPT_MAX]
        __attribute__((aligned(4)));
    // 中断描述符表
    static idt_entry_t idt_entries[INTERRUPT_MAX] __attribute__((aligned(16)));
    // IDTR
    static idt_ptr_t idt_ptr;

    // 系统中断
    static void divide_error(pt_regs_t *regs);
    static void debug(pt_regs_t *regs);
    static void nmi(pt_regs_t *regs);
    static void breakpoint(pt_regs_t *regs);
    static void overflow(pt_regs_t *regs);
    static void bound(pt_regs_t *regs);
    static void invalid_opcode(pt_regs_t *regs);
    static void device_not_available(pt_regs_t *regs);
    static void double_fault(pt_regs_t *regs);
    static void coprocessor_error(pt_regs_t *regs);
    static void invalid_TSS(pt_regs_t *regs);
    static void segment_not_present(pt_regs_t *regs);
    static void stack_segment(pt_regs_t *regs);
    static void general_protection(pt_regs_t *regs);
    static void page_fault(pt_regs_t *regs);

    void die(const char *str, uint32_t oesp, uint32_t int_no) {
        // uint32_t * old_esp = (uint32_t *)oesp;
        pt_regs_t *old_esp = (pt_regs_t *)oesp;
        io.printf("%s\t: %d\n\r", str, int_no);
        io.printf("die_Unuseable.\n");
        // cs::EIP
        // oss:oesp
        // printk_color(red, "EIP:\t%08x:%p\nEFLAGS:\t%08x\nESP:\t%08x:%p\n",
        //              old_esp[1], read_eflags(), read_eflags(), old_esp[4],
        //              old_esp[3]);
        // printk_color(red, "EIP:\t%08x:%p\nEFLAGS:\t%08x\nESP:\t%08x:%p\n",
        //              &old_esp[1], &old_esp[0], &old_esp[2], &old_esp[4],
        //              &old_esp[3]);
        // printk_color(red,
        // "EIP:\t%08x:%08X\nEFLAGS:\t%08x\nESP:\t%08x:%08X\n", old_esp->cs,
        // old_esp->eip, old_esp->eflags, old_esp->ss, old_esp->old_esp);
        io.printf("gs: %08x\tfs: %08x\tes: %08x\tds: %08x\n", old_esp->gs,
                  old_esp->fs, old_esp->es, old_esp->ds);
        io.printf("edi: %08x\tesi: %08x\tebp: %08x\told_esp: %08x\n",
                  old_esp->edi, old_esp->esi, old_esp->ebp, old_esp->old_esp);
        io.printf("ebx: %08x\tedx: %08x\tecx: %08x\teax: %08x\n", old_esp->ebx,
                  old_esp->edx, old_esp->ecx, old_esp->eax);
        io.printf("int_no: %08X\terr_code: %08X\teip: %08x\tcs: %08x\n",
                  old_esp->int_no, old_esp->err_code, old_esp->eip,
                  old_esp->cs);
        io.printf("eflags: %08x\tuser_esp: %08x\tss: %08x\n", old_esp->eflags,
                  old_esp->user_esp, old_esp->user_ss);
        io.printf("addr: %08x, %08X\n", &old_esp->gs, &old_esp->user_ss);

        cpu_hlt();
        return;
    }

    static void divide_error(pt_regs_t *regs) {
        die("Divide Error.", regs->old_esp, regs->int_no);
        return;
    }

    static void debug(pt_regs_t *regs) {
        uint32_t  tr;
        uint32_t *old_esp = (uint32_t *)regs->old_esp;

        // 取任务寄存器值->tr
        __asm__ volatile("str %%ax" : "=a"(tr) : "0"(0));
        io.printf("Unuseable.\n");

        io.printf("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n", regs->eax,
                  regs->ebx, regs->ecx, regs->edx);
        io.printf("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                  "esi 0x%08X\tedi 0x%08X\tebp 0x%08X\tesp 0x%08X\n", regs->esi,
                  regs->edi, regs->ebp, (uint32_t)regs->user_esp);
        io.printf("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                  "ds 0x%08X\tes 0x%08X\tfs 0x%08X\tgs 0x%08X\n", regs->ds,
                  regs->es, regs->fs, regs->gs);
        io.printf("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                  "EIP: 0x%08X\tEFLAGS: 0x%08X\tCS: 0x%08X\n",
                  // old_esp[0], old_esp[1], old_esp[2]);
                  old_esp[0], read_eflags(), old_esp[2]);
        return;
    }

    static void nmi(pt_regs_t *regs) {
        die("NMI.", regs->old_esp, regs->int_no);
        return;
    }

    static void breakpoint(pt_regs_t *regs) {
        die("Breakpoint.", regs->old_esp, regs->int_no);
        return;
    }

    static void overflow(pt_regs_t *regs) {
        die("Overflow.", regs->old_esp, regs->int_no);
        return;
    }

    static void bound(pt_regs_t *regs) {
        die("Bound.", regs->old_esp, regs->int_no);
        return;
    }

    static void invalid_opcode(pt_regs_t *regs) {
        die("Invalid Opcode.", regs->old_esp, regs->int_no);
        return;
    }

    static void device_not_available(pt_regs_t *regs) {
        die("Device Not Available.", regs->old_esp, regs->int_no);
        return;
    }

    static void double_fault(pt_regs_t *regs) {
        die("Double Fault.", regs->old_esp, regs->int_no);
        return;
    }

    static void coprocessor_error(pt_regs_t *regs) {
        die("Coprocessor Error.", regs->old_esp, regs->int_no);
        return;
    }

    static void invalid_TSS(pt_regs_t *regs) {
        die("Invalid TSS.", regs->old_esp, regs->int_no);
        return;
    }

    static void segment_not_present(pt_regs_t *regs) {
        die("Segment Not Present.", regs->old_esp, regs->int_no);
        return;
    }

    static void stack_segment(pt_regs_t *regs) {
        die("Stack Segment.", regs->old_esp, regs->int_no);
        return;
    }

    static void general_protection(pt_regs_t *regs) {
        die("General Protection.", regs->old_esp, regs->int_no);
        return;
    }

    static void page_fault(pt_regs_t *regs) {
#ifdef __x86_64__
        uint64_t cr2;
        asm volatile("movq %%cr2,%0" : "=r"(cr2));
#else
        uint32_t cr2;
        asm volatile("mov %%cr2,%0" : "=r"(cr2));
#endif
        io.printf("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                  "Page fault at 0x%08X, virtual faulting address 0x%08X\n",
                  regs->eip, cr2);
        io.printf("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                  "Error code: 0x%08X\n", regs->err_code);

        // bit 0 为 0 指页面不存在内存里
        if (!(regs->err_code & 0x1)) {
            io.printf("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                      "Because the page wasn't present.\n");
        }
        // bit 1 为 0 表示读错误，为 1 为写错误
        if (regs->err_code & 0x2) {
            io.printf("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                      "Write error.\n");
        }
        else {
            io.printf("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                      "Read error.\n");
        }
        // bit 2 为 1 表示在用户模式打断的，为 0 是在内核模式打断的
        if (regs->err_code & 0x4) {
            io.printf("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                      "In user mode.\n");
        }
        else {
            io.printf("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                      "In kernel mode.\n");
        }
        // bit 3 为 1 表示错误是由保留位覆盖造成的
        if (regs->err_code & 0x8) {
            io.printf("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                      "Reserved bits being overwritten.\n");
        }
        // bit 4 为 1 表示错误发生在取指令的时候
        if (regs->err_code & 0x10) {
            io.printf("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                      "The fault occurred during an instruction fetch.\n");
        }
        while (1) {
            ;
        }
        return;
    }

    void set_idt(uint8_t num, uint32_t base, uint16_t target, uint8_t flags) {
        idt_entries[num].base_low  = (base & 0xFFFF);
        idt_entries[num].base_high = (base >> 16) & 0xFFFF;
        idt_entries[num].selector  = target;
        idt_entries[num].zero      = 0;
        idt_entries[num].flags     = flags;
        // 0x8E: DPL=0
        // 0xEF: DPL=3
        return;
    }

    const char *get_intr_name(uint32_t intrno) {
        if (intrno < sizeof(intrnames) / sizeof(const char *const)) {
            return intrnames[intrno];
        }
        return "(unknown trap)";
    }

    void register_interrupt_handler(uint8_t n, interrupt_handler_t h) {
        interrupt_handlers[n] = h;
        return;
    }

    void enable_irq(uint32_t irq_no) {
        uint8_t mask = 0;
        // printk_color(green, "enable_irq mask: %X", mask);
        if (irq_no >= IRQ8) {
            mask = ((io.inb(IO_PIC2C)) & (~(1 << (irq_no % 8))));
            io.outb(IO_PIC2C, mask);
        }
        else {
            mask = ((io.inb(IO_PIC1C)) & (~(1 << (irq_no % 8))));
            io.outb(IO_PIC1C, mask);
        }
        return;
    }

    void disable_irq(uint32_t irq_no) {
        uint8_t mask = 0;
        // printk_color(green, "disable_irq mask: %X", mask);
        if (irq_no >= IRQ8) {
            mask = ((io.inb(IO_PIC2C)) | (1 << (irq_no % 8)));
            io.outb(IO_PIC2C, mask);
        }
        else {
            mask = ((io.inb(IO_PIC1C)) | (1 << (irq_no % 8)));
            io.outb(IO_PIC1C, mask);
        }
        return;
    }

    uint32_t get_irq(uint32_t irq_no) {
        uint32_t res = 0;
        switch (irq_no) {
            case 0: {
                res = IRQ0;
                break;
            }
            case 1: {
                res = IRQ1;
                break;
            }
            case 2: {
                res = IRQ2;
                break;
            }
            case 3: {
                res = IRQ3;
                break;
            }
            case 4: {
                res = IRQ4;
                break;
            }
            case 5: {
                res = IRQ5;
                break;
            }
            case 6: {
                res = IRQ6;
                break;
            }
            case 7: {
                res = IRQ7;
                break;
            }
            case 8: {
                res = IRQ8;
                break;
            }
            case 9: {
                res = IRQ9;
                break;
            }
            case 10: {
                res = IRQ10;
                break;
            }
            case 11: {
                res = IRQ11;
                break;
            }
            case 12: {
                res = IRQ12;
                break;
            }
            case 13: {
                res = IRQ13;
                break;
            }
            case 14: {
                res = IRQ14;
                break;
            }
            case 15: {
                res = IRQ15;
                break;
            }
        }
        return res;
    }

    void init_interrupt_chip(void) {
        // 重新映射 IRQ 表
        // 两片级联的 Intel 8259A 芯片
        // 主片端口 0x20 0x21
        // 从片端口 0xA0 0xA1

        // 初始化主片、从片
        // 0001 0001
        io.outb(IO_PIC1, 0x11);
        // 设置主片 IRQ 从 0x20(32) 号中断开始
        io.outb(IO_PIC1C, get_irq(0));
        // 设置主片 IR2 引脚连接从片
        io.outb(IO_PIC1C, 0x04);
        // 设置主片按照 8086 的方式工作
        io.outb(IO_PIC1C, 0x01);

        io.outb(IO_PIC2, 0x11);
        // 设置从片 IRQ 从 0x28(40) 号中断开始
        io.outb(IO_PIC2C, get_irq(8));
        // 告诉从片输出引脚和主片 IR2 号相连
        io.outb(IO_PIC2C, 0x02);
        // 设置从片按照 8086 的方式工作
        io.outb(IO_PIC2C, 0x01);

        // 默认关闭所有中断
        io.outb(IO_PIC1C, 0xFF);
        io.outb(IO_PIC2C, 0xFF);
        return;
    }

    void clear_interrupt_chip(uint32_t intr_no) {
        // 发送中断结束信号给 PICs
        // 按照我们的设置，从 32 号中断起为用户自定义中断
        // 因为单片的 Intel 8259A 芯片只能处理 8 级中断
        // 故大于等于 40 的中断号是由从片处理的
        if (intr_no >= get_irq(8)) {
            // 发送重设信号给从片
            io.outb(IO_PIC2, PIC_EOI);
        }
        // 发送重设信号给主片
        io.outb(IO_PIC1, PIC_EOI);
        return;
    }

    int32_t call_irq(uint32_t intr_no, pt_regs_t *regs) {
        // 重设PIC芯片
        clear_interrupt_chip(intr_no);
        if (interrupt_handlers[intr_no] != nullptr) {
            interrupt_handlers[intr_no](regs);
        }
        return 0;
    }

    int32_t call_isr(uint32_t intr_no, pt_regs_t *regs) {
        if (interrupt_handlers[intr_no] != nullptr) {
            interrupt_handlers[intr_no](regs);
        }
        else {
            io.printf("eax 0x%08X\tebx 0x%08X\tecx 0x%08X\tedx 0x%08X\n",
                      "Unhandled interrupt: %d %s\n", intr_no,
                      get_intr_name(intr_no));
            cpu_hlt();
        }
        return 0;
    }

    int32_t init(void) {
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
        io.printf("intr_init\n");
        return 0;
    }

};
