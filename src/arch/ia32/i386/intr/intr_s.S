
// This file is a part of Simple-XX/SimpleKernel (https://github.com/Simple-XX/SimpleKernel).
#
// intr_s.s for Simple-XX/SimpleKernel.

.code32

// 加载 idt
.section .text
.global idt_load
idt_load:
    // 参数保存在 eax
    mov 4(%esp), %eax
    lidt 0(%eax)
    ret

// 定义两个构造中断处理函数的宏(有的中断有错误代码，有的没有)
// 用于没有错误代码的中断
.macro ISR_NO_ERROR_CODE no
.global isr\no
isr\no:
    // 关中断
    cli
    // 这时候栈的情况
    // EIP
    // CS
    // EFLAGS
    // old_esp(特权级切换时 cpu 压入)
    // old_ss(特权级切换时 cpu 压入)
    // 压入错误代码，但这里没有传递，所以用 0 占位
    push $0
    // 压入寄存器信息
    pusha
    // 压入段寄存器
    push %ds
    push %es
    push %fs
    push %gs
    push %ss
    // 传递参数 2：错误代码，但这里没有传递，所以用 0 占位
    push $0
    // 传递参数 1：intr_context_t
    push %esp
    // 传递参数 0：中断号
    push $\no
    // 调用处理函数
    call isr_handler
    // 恢复现场
    // 弹出中断号
    add $4, %esp
    // 弹出 intr_context_t
    add $4, %esp
    // 弹出占位的 0
    add $4, %esp
    // 弹出寄存器
    pop %ss
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa
    // 弹出占位的 0
    add $4, %esp
    iret
.endm

// 用于有错误代码的中断
.macro ISR_ERROR_CODE no
.global isr\no
isr\no:
    // 关中断
    cli
    // 这时候栈的情况
    // err_code
    // EIP
    // CS
    // EFLAGS
    // old_esp(特权级切换时 cpu 压入)
    // old_ss(特权级切换时 cpu 压入)
    // 压入寄存器信息
    pusha
    // 压入段寄存器
    push %ds
    push %es
    push %fs
    push %gs
    push %ss
    // 传递参数 2：错误代码，位于栈地址+52
    push 52(%esp)
    // 传递参数 1：intr_context_t
    push %esp
    // 传递参数 0：中断号
    push $\no
    // 调用处理函数
    call isr_handler
    // 恢复现场
    // 弹出参数 0：中断号
    add $4, %esp
    // 弹出参数 1： intr_context_t
    add $4, %esp
    // 弹出参数 2：错误代码
    add $4, %esp
    // 弹出寄存器
    pop %ss
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa
    // TODO: 不知道这个是为啥，没有的话就会挂
    // 怀疑是错误代码，iret 没有将它弹出
    add $4, %esp
    iret
.endm

// 定义中断处理函数
// 64-ia-32-architectures-software-developer-vol-3a-manual#Table 6-1
// 0 #DE 除 0 异常
ISR_NO_ERROR_CODE  0
// 1 #DB 调试异常
ISR_NO_ERROR_CODE  1
// 2 NMI
ISR_NO_ERROR_CODE  2
// 3 BP 断点异常
ISR_NO_ERROR_CODE  3
// 4 #OF 溢出
ISR_NO_ERROR_CODE  4
// 5 #BR 对数组的引用超出边界
ISR_NO_ERROR_CODE  5
// 6 #UD 无效或未定义的操作码
ISR_NO_ERROR_CODE  6
// 7 #NM 设备不可用(无数学协处理器)
ISR_NO_ERROR_CODE  7
// 8 #DF 双重故障(有错误代码)
ISR_ERROR_CODE    8
// 9 协处理器跨段操作
ISR_NO_ERROR_CODE  9
// 10 #TS 无效TSS(有错误代码)
ISR_ERROR_CODE   10
// 11 #NP 段不存在(有错误代码)
ISR_ERROR_CODE   11
// 12 #SS 栈错误(有错误代码)
ISR_ERROR_CODE   12
// 13 #GP 常规保护(有错误代码)
ISR_ERROR_CODE   13
// 14 #PF 页故障(有错误代码)
ISR_ERROR_CODE   14
// 15 没有使用
// 16 #MF 浮点处理单元错误
ISR_NO_ERROR_CODE 16
// 17 #AC 对齐检查
ISR_ERROR_CODE   17
// 18 #MC 机器检查
ISR_NO_ERROR_CODE 18
// 19 #XM SIMD(单指令多数据)浮点异常
ISR_NO_ERROR_CODE 19
// 20 #VE 虚拟化异常
ISR_NO_ERROR_CODE 20
// 21 ~ 31 保留
// 32 ～ 255 用户自定义
// 128=0x80 用于系统调用
ISR_NO_ERROR_CODE 128

// 构造中断请求的宏
.macro IRQ name, no
.global irq\name
irq\name:
    // 关中断
    cli
    // 这时候栈的情况
    // EIP
    // CS
    // EFLAGS
    // old_esp(特权级切换时 cpu 压入)
    // old_ss(特权级切换时 cpu 压入)
    // 压入错误代码，但这里没有传递，所以用 0 占位
    push $0
    // 压入寄存器信息
    pusha
    // 压入段寄存器
    push %ds
    push %es
    push %fs
    push %gs
    push %ss
    // 传递参数 1：intr_context_t
    push %esp
    // 传递参数 0：中断号
    push $\no
    // 调用处理函数
    call irq_handler
    // 恢复现场
    // 弹出中断号
    add $4, %esp
    // 弹出 intr_context_t
    add $4, %esp
    // 弹出寄存器
    pop %ss
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa
    # 弹出占位的 0
    add $4, %esp
    iret
.endm

// 电脑系统计时器
IRQ   0,    32
// 键盘
IRQ   1,    33
// 与 IRQ9 相接，MPU-401 MD 使用
IRQ   2,    34
// 串口设备
IRQ   3,    35
// 串口设备
IRQ   4,    36
// 建议声卡使用
IRQ   5,    37
// 软驱传输控制使用
IRQ   6,    38
// 打印机传输控制使用
IRQ   7,    39
// 即时时钟
IRQ   8,    40
// 与 IRQ2 相接，可设定给其他硬件
IRQ   9,    41
// 建议网卡使用
IRQ  10,    42
// 建议 AGP 显卡使用
IRQ  11,    43
// 接 PS/2 鼠标，也可设定给其他硬件
IRQ  12,    44
// 协处理器使用
IRQ  13,    45
// IDE0 传输控制使用
IRQ  14,    46
// IDE1 传输控制使用
IRQ  15,    47
