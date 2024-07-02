
/**
 * @file interrupt_s.S
 * @brief x86_64 中断相关汇编代码
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2024-04-24
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2024-04-24<td>Zone.N (Zone.Niuzh@hotmail.com)<td>创建文件
 * </table>
 */

#include "macro.S"

// clang-format off

// 声明 C 中的函数
.extern TarpEntry

/**
 * @brief 处理无错误码的中断
 * @param name 函数名
 * @param no 错误号
 */
.macro IsrNoErrorCode name no
.global IsrNoErrorCode_\name
.type IsrNoErrorCode_\name, @function
IsrNoErrorCode_\name:
    // 关中断
    cli
    // 保存现场
    pusha
    // 传递中断号
    mov $\no, %rdi
    // 传递栈地址
    mov %rsp, %rsi
    // 清零
    xor %rdx, %rdx
    xor %rbp, %rbp
    // 调用处理函数
    call TarpEntry
    // 恢复现场
    popa
    // 中断返回
    iretq
.endm

/**
 * @brief 处理有错误码的中断
 * @param name 函数名
 * @param no 错误号
 */
.macro IsrErrorCode name no
.global IsrErrorCode_\name
.type IsrErrorCode_\name, @function
IsrErrorCode_\name:
    // 关中断
    cli
    // 压栈 intr_context_t 的最后几个成员
    // 40(%rsp) 是因为每次压栈都会使 rsp+8，执行完后 rsp 刚好移动到下一个数据
    // 在 64 位模式下，不论特权级是否改变，都会压入 ss，rsp
    // 64-ia-32-architectures-software-developer-vol-3a-manual#6.14.2
    // SS
    push 40(%rsp)
    // RSP
    push 40(%rsp)
    // RFLAGS
    push 40(%rsp)
    // CS
    push 40(%rsp)
    // RIP
    push 40(%rsp)
    pusha
    // 传递中断号
    mov $\no, %rdi
    // 传递栈地址
    mov %rsp, %rsi
    // 传递错误码
    // 160 == sizeof(intr_context_t)
    // 相当于函数开始时的 0(%rsp)
    mov  160(%rsp), %rdx
    xor %rbp, %rbp
    call TarpEntry
    popa
    iretq
.endm

/**
 * @brief 中断请求处理
 * @param name 函数名
 * @param no irq 号
 */
.macro Irq name, no
.global Irq_\name
.type Irq_\name, @function
Irq_\name:
    // 关中断
    cli
    // 保存寄存器
    pusha
    // 传递中断号
    mov $\no, %rdi
    // 传递栈地址
    mov %rsp, %rsi
    xor %rdx, %rdx
    xor %rbp, %rbp
    call TarpEntry
    popa
    iretq
.endm

// 定义中断处理函数
// 64-ia-32-architectures-software-developer-vol-3a-manual#Table 6-1
// 0 #DE 除 0 异常
IsrNoErrorCode  DE, 0
// 1 #DB 调试异常
IsrNoErrorCode  DB, 1
// 2 NMI
IsrNoErrorCode NMI, 2
// 3 BP 断点异常
IsrNoErrorCode BP, 3
// 4 #OF 溢出
IsrNoErrorCode OF, 4
// 5 #BR 对数组的引用超出边界
IsrNoErrorCode BR, 5
// 6 #UD 无效或未定义的操作码
IsrNoErrorCode UD, 6
// 7 #NM 设备不可用(无数学协处理器)
IsrNoErrorCode NM, 7
// 8 #DF 双重故障(有错误代码)
IsrErrorCode DF, 8
// 10 #TS 无效TSS(有错误代码)
IsrErrorCode TS, 10
// 11 #NP 段不存在(有错误代码)
IsrErrorCode NP, 11
// 12 #SS 栈错误(有错误代码)
IsrErrorCode SS, 12
// 13 #GP 常规保护(有错误代码)
IsrErrorCode GP, 13
// 14 #PF 页故障(有错误代码)
IsrErrorCode PF, 14
// 16 #MF 浮点处理单元错误
IsrNoErrorCode MF, 16
// 17 #AC 对齐检查
IsrErrorCode AC, 17
// 18 #MC 机器检查
IsrNoErrorCode MC, 18
// 19 #XM SIMD(单指令多数据)浮点异常
IsrNoErrorCode XM, 19
// 20 #VE 虚拟化异常
IsrNoErrorCode VE, 20
// 21 ~ 31 保留
// 32 ～ 255 用户自定义
// 128 == 0x80 用于系统调用
IsrNoErrorCode SYCALL, 128

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
