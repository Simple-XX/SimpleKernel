
# This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
# intr_s.s for Simple-XX/SimpleKernel.

.code32

# 加载 idt
.section .text
.global idt_load
idt_load:
    # 参数保存在 eax
    mov 4(%esp), %eax
    lidt (%eax)
    ret

# 定义两个构造中断处理函数的宏(有的中断有错误代码，有的没有)
# 用于没有错误代码的中断

.macro ISR_NOERRCODE no
.global isr\no
isr\no:
    # 首先关闭中断
    cli
    # push 无效的中断错误代码,占位用
    push $0
    # push 中断号
    push $\no
    jmp isr_common_stub
.endm

# 用于有错误代码的中断
.macro ISR_ERRCODE no
.global isr\no
isr\no:
    # 关闭中断
    cli
    # push 中断号
    push $\no
    jmp isr_common_stub
.endm

# 定义中断处理函数
# 64-ia-32-architectures-software-developer-vol-3a-manual#Table 6-1
# 0 #DE 除 0 异常
ISR_NOERRCODE  0
# 1 #DB 调试异常
ISR_NOERRCODE  1
# 2 NMI
ISR_NOERRCODE  2
# 3 BP 断点异常
ISR_NOERRCODE  3
# 4 #OF 溢出
ISR_NOERRCODE  4
# 5 #BR 对数组的引用超出边界
ISR_NOERRCODE  5
# 6 #UD 无效或未定义的操作码
ISR_NOERRCODE  6
# 7 #NM 设备不可用(无数学协处理器)
ISR_NOERRCODE  7
# 8 #DF 双重故障(有错误代码)
ISR_ERRCODE    8
# 9 协处理器跨段操作
ISR_NOERRCODE  9
# 10 #TS 无效TSS(有错误代码)
ISR_ERRCODE   10
# 11 #NP 段不存在(有错误代码)
ISR_ERRCODE   11
# 12 #SS 栈错误(有错误代码)
ISR_ERRCODE   12
# 13 #GP 常规保护(有错误代码)
ISR_ERRCODE   13
# 14 #PF 页故障(有错误代码)
ISR_ERRCODE   14
# 15 没有使用
# 16 #MF 浮点处理单元错误
ISR_NOERRCODE 16
# 17 #AC 对齐检查
ISR_ERRCODE   17
# 18 #MC 机器检查
ISR_NOERRCODE 18
# 19 #XM SIMD(单指令多数据)浮点异常
ISR_NOERRCODE 19
# 20 #VE 虚拟化异常
ISR_NOERRCODE 20
# 21 ~ 31 保留
# 32 ～ 255 用户自定义
# 128=0x80 用于系统调用
ISR_NOERRCODE 128

# 中断服务程序
.global isr_common_stub
.extern isr_handler
isr_common_stub:
    pusha
    push %ds
    push %es
    push %fs
    push %gs

    # 加载内核数据段描述符表, 0x10:内核数据段标识符
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    push %esp
    call isr_handler
    # 清除压入的参数
    add $4, %esp
    call forkret_s

# 构造中断请求的宏
.macro IRQ name, no
.global irq\name
irq\name:
    cli
    push $0
    push $\no
    jmp irq_common_stub
.endm

# 电脑系统计时器
IRQ   0,    32
# 键盘
IRQ   1,    33
# 与 IRQ9 相接，MPU-401 MD 使用
IRQ   2,    34
# 串口设备
IRQ   3,    35
# 串口设备
IRQ   4,    36
# 建议声卡使用
IRQ   5,    37
# 软驱传输控制使用
IRQ   6,    38
# 打印机传输控制使用
IRQ   7,    39
# 即时时钟
IRQ   8,    40
# 与 IRQ2 相接，可设定给其他硬件
IRQ   9,    41
# 建议网卡使用
IRQ  10,    42
# 建议 AGP 显卡使用
IRQ  11,    43
# 接 PS/2 鼠标，也可设定给其他硬件
IRQ  12,    44
# 协处理器使用
IRQ  13,    45
# IDE0 传输控制使用
IRQ  14,    46
# IDE1 传输控制使用
IRQ  15,    47

.global irq_common_stub
.extern irq_handler
irq_common_stub:
    pusha
    push %ds
    push %es
    push %fs
    push %gs

    # 加载内核数据段描述符表, 0x10:内核数据段标识符
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    push %esp
    call irq_handler
    # 清除压入的参数
    add $0x04, %esp

.global forkret_s
forkret_s:
    pop %gs
    pop %fs
    pop %es
    pop %ds
    # Pops edi,esi,ebp...
    popa
    # 清理压栈的 错误代码 和 ISR 编号
    add $0x08, %esp
    # 出栈 CS, EIP, EFLAGS, SS, ESP
    iret
