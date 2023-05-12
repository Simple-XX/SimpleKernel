
/**
 * @file cpu.hpp
 * @brief cpu 相关定义
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-09-18<td>digmouse233<td>迁移到 doxygen
 * </table>
 */

#ifndef SIMPLEKERNEL_CPU_HPP
#define SIMPLEKERNEL_CPU_HPP

#include "cstdbool"
#include "cstdint"
#include "cstdio"

/**
 * @brief cpu 相关
 * @todo CPUID 相关操作，补全寄存器操作，数据地址等
 */
namespace CPU {
// CR0：包含当前处理器运行的控制标志。
// CR1：保留。
// CR2：包含发生页面错误时的线性地址。
// CR3：页面目录表（Page Directory Table）的物理地址。
// 虚拟地址启用且CR0中PG位设置为1的情况下，CR3可以协助处理器将线性地址转换为物理地址。一般情况下为MMU提供页表的入口实现。
// CR4：包含处理器扩展功能的标志位。
// CR8：提供对任务优先级寄存器（Task Priority
// Register）的读写（仅在64位模式下存在）。 对控制寄存器的读写是通过MOV
// CRn指令来实现

/// PE：CR0的位0是启用保护（Protection
// Enable）标志。当设置该位时即开启了保护模式；
// 当复位时即进入实地址模式。这个标志仅开启段级保护，而并没有启用分页机制。若要启用分页机制，那么PE和PG标志都要置位。
static constexpr const uint32_t CR0_PE = 0x00000001;
static constexpr const uint32_t CR0_MP = 0x00000002;
static constexpr const uint32_t CR0_EM = 0x00000004;
static constexpr const uint32_t CR0_TS = 0x00000008;
static constexpr const uint32_t CR0_ET = 0x00000010;

/// NE：对于 Intel 80486或以上的CPU，CR0 的位5是协处理器错误（Numeric
// Error）标志。 当设置该标志时，就启用了 x87
// 协处理器错误的内部报告机制；若复位该标志，那么就使用 PC 形式的 x87
// 协处理器错误报告机制。 当NE为复位状态并且 CPU 的 IGNNE
// 输入引脚有信号时，那么数学协处理器 x87 错误将被忽略。 当NE为复位状态并且
// CPU 的 IGNNE 输入引脚无信号时，那么非屏蔽的数学协处理器 x87
// 错误将导致处理器通过 FERR 引脚在外部产生一个中断，
// 并且在执行下一个等待形式浮点指令或 WAIT/FWAIT 指令之前立刻停止指令执行。
// CPU 的 FERR 引脚用于仿真外部协处理器 80387 的 ERROR 引脚，
// 因此通常连接到中断控制器输入请求引脚上。NE 标志、IGNNE 引脚和 FERR
// 引脚用于利用外部逻辑来实现 PC 形式的外部错误报告机制。
static constexpr const uint32_t CR0_NE = 0x00000020;

/// WP：对于Intel 80486或以上的CPU，CR0的位16是写保护（Write
/// Proctect）标志。
// 当设置该标志时，处理器会禁止超级用户程序（例如特权级0的程序）向用户级只读页面执行写操作；当该位复位时则反之。该标志有利于UNIX类操作系统在创建进程时实现写时复制（Copy
// on Write）技术。
static constexpr const uint32_t CR0_WP = 0x00010020;
static constexpr const uint32_t CR0_AM = 0x00040020;
static constexpr const uint32_t CR0_NW = 0x20000000;
static constexpr const uint32_t CR0_CD = 0x40000000;

/// PG：CR0的位31是分页（Paging）标志。当设置该位时即开启了分页机制；
// 当复位时则禁止分页机制，此时所有线性地址等同于物理地址。在开启这个标志之前必须已经或者同时开启PE标志。即若要启用分页机制，那么PE和PG标志都要置位。
static constexpr const uint32_t CR0_PG = 0x80000000;

/// 启用保护模式PE（Protected
// Enable）位（位0）和开启分页PG（Paging）位（位31）分别用于控制分段和分页机制。
// PE用于控制分段机制。如果PE=1，处理器就工作在开启分段机制环境下，即运行在保护模式下。
// 如果PE=0，则处理器关闭了分段机制，并如同8086工作于实地址模式下。PG用于控制分页机制。如果PG=1，则开启了分页机制。
// 如果PG=0，分页机制被禁止，此时线性地址被直接作为物理地址使用。
// 如果PE=0、PG=0，处理器工作在实地址模式下；如果PG=0、PE=1，处理器工作在没有开启分页机制的保护模式下；
// 如果PG=1、PE=0，此时由于不在保护模式下不能启用分页机制，因此处理器会产生一个一般保护异常，即这种标志组合无效；
// 如果PG=1、PE=1，则处理器工作在开启了分页机制的保护模式下。

// 段描述符 DPL
/// 内核级
static constexpr const uint32_t DPL0   = 0x00;
static constexpr const uint32_t DPL1   = 0x01;
static constexpr const uint32_t DPL2   = 0x02;
/// 用户级
static constexpr const uint32_t DPL3   = 0x03;

/**
 * @brief 执行CPU空操作
 */
inline static void              hlt(void) {
    __asm__ volatile("hlt");
    return;
}

/**
 * @brief 开启中断
 */
inline static void sti(void) {
    __asm__ volatile("sti" ::: "memory");
    return;
}

/**
 * @brief 关闭中断
 */
inline static void cli(void) {
    __asm__ volatile("cli" ::: "memory");
    return;
}

/**
 * @brief 出发 debug 中断
 */
inline static void debug_intr(void) {
    __asm__ volatile("int $0x01");
    return;
}

/**
 * @brief 读取 EFLAGS
 * @return uint32_t        eflags 值
 */
inline static uint32_t read_eflags(void) {
    uint32_t eflags;
    __asm__ volatile("pushf\n\t"
                     "pop %0\n\t"
                     : "=r"(eflags));
    return eflags;
}

/**
 * @brief 读取 CR0
 * @return uint32_t        CR0 值
 */
inline static uint32_t read_cr0(void) {
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=b"(cr0));
    return cr0;
}

/**
 * @brief 读取 CR2
 * @return uint32_t        CR2 值
 */
inline static uint32_t read_cr2(void) {
    uint32_t cr2;
    __asm__ volatile("mov %%cr2, %0" : "=b"(cr2));
    return cr2;
}

/**
 * @brief 读取 CR3
 * @return uint32_t        CR3 值
 */
inline static uint32_t read_cr3(void) {
    uint32_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=b"(cr3));
    return cr3;
}

/**
 * @brief 切换内核栈
 * @param  _stack_top       要切换的栈顶地址
 */
inline static void switch_stack(void* _stack_top) {
    asm("mov %0, %%esp" : : "r"(_stack_top));
    asm("xor %%ebp, %%ebp" : :);
    return;
}

/**
 * @brief 读取 CR4
 * @return uint32_t        CR4 值
 */
inline static uint32_t read_cr4(void) {
    uint32_t cr4;
    __asm__ volatile("mov %%cr4, %0" : "=b"(cr4));
    return cr4;
}

/**
 * @brief 刷新页表缓存
 * @param  _addr            要刷新的地址
 */
inline static void INVLPG(void* _addr) {
    __asm__ volatile("invlpg (%0)" : : "r"(_addr) : "memory");
    return;
}

// 开启 PG
inline static bool ENABLE_PG(void) {
    uintptr_t cr0 = 0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    // 最高位 PG 位置 1，分页开启
    cr0 |= (1u << 31);
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
    info("paging enabled.\n");
    return true;
}

/**
 * @brief 页表结构 32 位分页
 * @note Use of CR3 with 32-Bit Paging
 */
class cr3_t {
public:
    // Ignored
    uint32_t ignore1 : 3;
    // Page-level write-through; indirectly determines the memory type used
    // to access the page directory during linear-address translation
    uint32_t pwt     : 1;
    // Page-level cache disable; indirectly determines the memory type used
    // to access the page directory during linear-address translation
    uint32_t pcd     : 1;
    // Ignored
    uint32_t ignore2 : 7;
    // Physical address of the 4-KByte aligned page directory used for
    // linear-address translation
    uint32_t addr    : 20;
};

/**
 * @brief 设置 页目录
 * @param  _pgd            要设置的页表
 * @return true            成功
 * @return false           失败
 */
inline static bool SET_PGD(uintptr_t _pgd) {
    __asm__ volatile("mov %0, %%cr3" : : "r"(_pgd));
    return true;
}

};     // namespace CPU

#endif /* SIMPLEKERNEL_CPU_HPP */
