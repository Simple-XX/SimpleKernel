
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
#include "cstring"

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
static constexpr const uint32_t CR0_PE                           = 0x00000001;
static constexpr const uint32_t CR0_MP                           = 0x00000002;
static constexpr const uint32_t CR0_EM                           = 0x00000004;
static constexpr const uint32_t CR0_TS                           = 0x00000008;
static constexpr const uint32_t CR0_ET                           = 0x00000010;

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
static constexpr const uint32_t CR0_NE                           = 0x00000020;

/// WP：对于Intel 80486或以上的CPU，CR0的位16是写保护（Write
/// Proctect）标志。
// 当设置该标志时，处理器会禁止超级用户程序（例如特权级0的程序）向用户级只读页面执行写操作；当该位复位时则反之。该标志有利于UNIX类操作系统在创建进程时实现写时复制（Copy
// on Write）技术。
static constexpr const uint32_t CR0_WP                           = 0x00010020;
static constexpr const uint32_t CR0_AM                           = 0x00040020;
static constexpr const uint32_t CR0_NW                           = 0x20000000;
static constexpr const uint32_t CR0_CD                           = 0x40000000;

/// PG：CR0的位31是分页（Paging）标志。当设置该位时即开启了分页机制；
// 当复位时则禁止分页机制，此时所有线性地址等同于物理地址。在开启这个标志之前必须已经或者同时开启PE标志。即若要启用分页机制，那么PE和PG标志都要置位。
static constexpr const uint32_t CR0_PG                           = 0x80000000;

/// 启用保护模式PE（Protected
// Enable）位（位0）和开启分页PG（Paging）位（位31）分别用于控制分段和分页机制。
// PE用于控制分段机制。如果PE=1，处理器就工作在开启分段机制环境下，即运行在保护模式下。
// 如果PE=0，则处理器关闭了分段机制，并如同8086工作于实地址模式下。PG用于控制分页机制。如果PG=1，则开启了分页机制。
// 如果PG=0，分页机制被禁止，此时线性地址被直接作为物理地址使用。
// 如果PE=0、PG=0，处理器工作在实地址模式下；如果PG=0、PE=1，处理器工作在没有开启分页机制的保护模式下；
// 如果PG=1、PE=0，此时由于不在保护模式下不能启用分页机制，因此处理器会产生一个一般保护异常，即这种标志组合无效；
// 如果PG=1、PE=1，则处理器工作在开启了分页机制的保护模式下。

static constexpr const uint32_t CR3_PWT                          = 0x00000008;
static constexpr const uint32_t CR3_PCD                          = 0x00000010;

static constexpr const uint32_t CR4_VME                          = 0x00000001;
static constexpr const uint32_t CR4_PVI                          = 0x00000002;
static constexpr const uint32_t CR4_TSD                          = 0x00000004;
static constexpr const uint32_t CR4_DE                           = 0x00000008;
// PSE 为 1 时，32bits 的页面大小变为 4MB
// 32bits 模式的分页模式支持物理地址宽度超过
// 32bit，这种情况下的页面大小必须是 4MB,并且处理器必须支持 PSE-3
static constexpr const uint32_t CR4_PSE                          = 0x00000010;
// 物理地址拓展
// 32bits 下寻址变为三级
static constexpr const uint32_t CR4_PAE                          = 0x00000020;
static constexpr const uint32_t CR4_MCE                          = 0x00000040;
static constexpr const uint32_t CR4_PGE                          = 0x00000080;
static constexpr const uint32_t CR4_PCE                          = 0x00000100;
static constexpr const uint32_t CR4_OSFXSR                       = 0x00000200;
static constexpr const uint32_t CR4_OSXMMEXCPT                   = 0x00000400;
static constexpr const uint32_t CR4_VMXE                         = 0x00002000;
static constexpr const uint32_t CR4_SMXE                         = 0x00004000;
static constexpr const uint32_t CR4_PCIDE                        = 0x00020000;
static constexpr const uint32_t CR4_OSXSAVE                      = 0x00040000;
static constexpr const uint32_t CR4_SMEP                         = 0x00100000;

static constexpr const uint32_t IA32_APIC_BASE                   = 0x1B;
static constexpr const uint32_t IA32_APIC_BASE_BSP_BIT           = 1 << 8;
static constexpr const uint32_t IA32_APIC_BASE_X2APIC_ENABLE_BIT = 1 << 10;
static constexpr const uint32_t IA32_APIC_BASE_GLOBAL_ENABLE_BIT = 1 << 11;

// x2APIC ID Register (R/O)
static constexpr const uint32_t IA32_X2APIC_APICID               = 0x802;
// x2APIC Version Register (R/O)
static constexpr const uint32_t IA32_X2APIC_VERSION              = 0x803;
// Support for EOI-broadcast suppression
static constexpr const uint32_t IA32_X2APIC_VERSION_EOI_SUPPORT_BIT = 1 << 24;
// x2APIC Task Priority Register (R/W)
static constexpr const uint32_t IA32_X2APIC_TPR                     = 0x808;
// x2APIC Processor Priority Register (R/O)
static constexpr const uint32_t IA32_X2APIC_PPR                     = 0x80A;
// x2APIC EOI Register (W/O)
static constexpr const uint32_t IA32_X2APIC_EOI                     = 0x80B;
// x2APIC Logical Destination Register (R/O)
static constexpr const uint32_t IA32_X2APIC_LDR                     = 0x80D;
// x2APIC Spurious Interrupt Vector Register (R/W)
static constexpr const uint32_t IA32_X2APIC_SIVR                    = 0x80F;
// 伪中断向量号
static constexpr const uint32_t IA32_X2APIC_SIVR_NO_BIT             = 0x0;
// APIC 软件使能 1:enable
static constexpr const uint32_t IA32_X2APIC_SIVR_APIC_ENABLE_BIT    = 1 << 8;
// 焦点处理器 0:disable
static constexpr const uint32_t IA32_X2APIC_SIVR_FOCUS_ENABLE_BIT   = 1 << 9;
// 禁止广播 EOI 消息使能 1:enable
static constexpr const uint32_t IA32_X2APIC_SIVR_EOI_ENABLE_BIT     = 1 << 12;
// x2APIC In-Service Register Bits 31:0 (R/O)
static constexpr const uint32_t IA32_X2APIC_ISR0                    = 0x810;
static constexpr const uint32_t IA32_X2APIC_ISR1                    = 0x811;
static constexpr const uint32_t IA32_X2APIC_ISR2                    = 0x812;
static constexpr const uint32_t IA32_X2APIC_ISR3                    = 0x813;
static constexpr const uint32_t IA32_X2APIC_ISR4                    = 0x814;
static constexpr const uint32_t IA32_X2APIC_ISR5                    = 0x815;
static constexpr const uint32_t IA32_X2APIC_ISR6                    = 0x816;
// x2APIC In-Service Register Bits 255:224 (R/O)
static constexpr const uint32_t IA32_X2APIC_ISR7                    = 0x817;
// x2APIC Trigger Mode Register Bits 31:0 (R/O)
static constexpr const uint32_t IA32_X2APIC_TMR0                    = 0x818;
static constexpr const uint32_t IA32_X2APIC_TMR1                    = 0x819;
static constexpr const uint32_t IA32_X2APIC_TMR2                    = 0x81A;
static constexpr const uint32_t IA32_X2APIC_TMR3                    = 0x81B;
static constexpr const uint32_t IA32_X2APIC_TMR4                    = 0x81C;
static constexpr const uint32_t IA32_X2APIC_TMR5                    = 0x81D;
static constexpr const uint32_t IA32_X2APIC_TMR6                    = 0x81E;
// x2APIC Trigger Mode Register Bits 255:224 (R/O)
static constexpr const uint32_t IA32_X2APIC_TMR7                    = 0x81F;
// x2APIC Interrupt Request Register Bits 31:0 (R/O)
static constexpr const uint32_t IA32_X2APIC_IRR0                    = 0x820;
static constexpr const uint32_t IA32_X2APIC_IRR1                    = 0x821;
static constexpr const uint32_t IA32_X2APIC_IRR2                    = 0x822;
static constexpr const uint32_t IA32_X2APIC_IRR3                    = 0x823;
static constexpr const uint32_t IA32_X2APIC_IRR4                    = 0x824;
static constexpr const uint32_t IA32_X2APIC_IRR5                    = 0x825;
static constexpr const uint32_t IA32_X2APIC_IRR6                    = 0x826;
// x2APIC Interrupt Request Register Bits 255:224 (R/O)
static constexpr const uint32_t IA32_X2APIC_IRR7                    = 0x827;
// x2APIC Error Status Register (R/W)
static constexpr const uint32_t IA32_X2APIC_ESR                     = 0x828;
// x2APIC LVT Corrected Machine Check Interrupt Register (R/W)
static constexpr const uint32_t IA32_X2APIC_CMCI                    = 0x82F;
// x2APIC Interrupt Command Register (R/W)
static constexpr const uint32_t IA32_X2APIC_ICR                     = 0x830;
// x2APIC LVT Timer Interrupt Register (R/W)
static constexpr const uint32_t IA32_X2APIC_LVT_TIMER               = 0x832;
// x2APIC LVT Thermal Sensor Interrupt Register (R/W)
static constexpr const uint32_t IA32_X2APIC_LVT_THERMAL             = 0x833;
// x2APIC LVT Performance Monitor Interrupt Register (R/W)
static constexpr const uint32_t IA32_X2APIC_LVT_PMI                 = 0x834;
// x2APIC LVT LINT0 Register (R/W)
static constexpr const uint32_t IA32_X2APIC_LVT_LINT0               = 0x835;
// x2APIC LVT LINT1 Register (R/W)
static constexpr const uint32_t IA32_X2APIC_LVT_LINT1               = 0x836;
// x2APIC LVT Error Register (R/W)
static constexpr const uint32_t IA32_X2APIC_LVT_ERROR               = 0x837;
// Bits 0-7	The vector number
static constexpr const uint32_t IA32_X2APIC_LVT_NO_BIT              = 0x0;
// Bits 8-11 (reserved for timer)	100b if NMI
static constexpr const uint32_t IA32_X2APIC_LVT_NMI_BIT             = 1 << 8;
// Bit 12	Set if interrupt pending.
static constexpr const uint32_t IA32_X2APIC_LVT_PENDING_BIT         = 1 << 12;
// Bit 13 (reserved for timer)	Polarity, set is low triggered
static constexpr const uint32_t IA32_X2APIC_LVT_POLAR_BIT           = 1 << 13;
// Bit 14 (reserved for timer)	Remote IRR
static constexpr const uint32_t IA32_X2APIC_LVT_REMOTE_IRR_BIT      = 1 << 14;
// Bit 15 (reserved for timer)	trigger mode, set is level triggered
static constexpr const uint32_t IA32_X2APIC_LVT_TRIGGER_BIT         = 1 << 15;
// Bit 16	Set to mask
static constexpr const uint32_t IA32_X2APIC_LVT_MASK_BIT            = 1 << 16;
// Bits 17-31	Reserved

// x2APIC Initial Count Register(R/W)
static constexpr const uint32_t IA32_X2APIC_TIMER_INIT_COUNT        = 0x838;
// x2APIC Current Count Register (R/O)
static constexpr const uint32_t IA32_X2APIC_TIMER_CUR_COUNT         = 0x839;
// x2APIC Divide Configuration Register (R/W)
static constexpr const uint32_t IA32_X2APIC_DIV_CONF                = 0x83E;
// x2APIC Self IPI Register (W/O)
static constexpr const uint32_t IA32_X2APIC_SELF_IPI                = 0x83F;

// 段描述符 DPL
/// 内核级
static constexpr const uint32_t DPL0                                = 0x00;
static constexpr const uint32_t DPL1                                = 0x01;
static constexpr const uint32_t DPL2                                = 0x02;
/// 用户级
static constexpr const uint32_t DPL3                                = 0x03;

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
inline static void ENABLE_INTR(void) {
    __asm__ volatile("sti" ::: "memory");
    return;
}

/**
 * @brief 关闭中断
 */
inline static void DISABLE_INTR(void) {
    __asm__ volatile("cli" ::: "memory");
    return;
}

/**
 * @brief 触发 debug 中断
 */
inline static void debug_intr(void) {
    __asm__ volatile("int $0x01");
    return;
}

/**
 * @brief 读取 EFLAGS
 * @return uint32_t        eflags 值
 */
inline static uint32_t READ_EFLAGS(void) {
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
inline static uint32_t READ_CR0(void) {
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=b"(cr0));
    return cr0;
}

/**
 * @brief 读取 CR2
 * @return uint32_t        CR2 值
 */
inline static uint32_t READ_CR2(void) {
    uint32_t cr2;
    __asm__ volatile("mov %%cr2, %0" : "=b"(cr2));
    return cr2;
}

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

/**
 * @brief 获取页目录 CR3
 * @return uintptr_t        CR3 值
 */
inline static uintptr_t GET_PGD(void) {
    uintptr_t cr3;
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
inline static uint32_t READ_CR4(void) {
    uint32_t cr4;
    __asm__ volatile("mov %%cr4, %0" : "=b"(cr4));
    return cr4;
}

/**
 * @brief 刷新页表缓存
 * @param  _addr            要刷新的地址
 */
inline static void VMM_FLUSH(uintptr_t _addr) {
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
 * @brief 读 MSR
 * @param  _idx            要读的索引
 * @return uint64_t        读取的值
 */
inline static uint64_t READ_MSR(uint32_t _idx) {
    uint32_t low;
    uint32_t high;
    __asm__ volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(_idx));
    return ((uint64_t)high << 32) | low;
}

/**
 * @brief 写 MSR
 * @param  _idx            要写的索引
 * @param  _msr            要写的值
 */
inline static void WRITE_MSR(uint32_t _idx, uint64_t _msr) {
    uint32_t low  = _msr;
    uint32_t high = _msr >> 32;
    __asm__ volatile("wrmsr" : : "a"(low), "d"(high), "c"(_idx));
    return;
}

/// @todo 改为 static
class CPUID {
private:
    static constexpr const uint32_t VENDOR_LEN          = 16;
    static constexpr const uint32_t BRAND_LEN           = 50;
    static constexpr const uint32_t FAMILY_LEN          = 50;
    static constexpr const uint32_t MODEL_LEN           = 50;
    // Vendor-strings.
    static constexpr const char*    VENDOR_OLDAMD       = "AMDisbetter!";
    // early engineering samples of AMD K5 processor
    static constexpr const char*    VENDOR_AMD          = "AuthenticAMD";
    static constexpr const char*    VENDOR_INTEL        = "GenuineIntel";
    static constexpr const char*    VENDOR_Centaur      = "CentaurHauls";
    static constexpr const char*    VENDOR_OLDTRANSMETA = "TransmetaCPU";
    static constexpr const char*    VENDOR_TRANSMETA    = "GenuineTMx86";
    static constexpr const char*    VENDOR_CYRIX        = "CyrixInstead";
    static constexpr const char*    VENDOR_CENTAUR      = "CentaurHauls";
    static constexpr const char*    VENDOR_NEXGEN       = "NexGenDriven";
    static constexpr const char*    VENDOR_UMC          = "UMC UMC UMC ";
    static constexpr const char*    VENDOR_SIS          = "SiS SiS SiS ";
    static constexpr const char*    VENDOR_NSC          = "Geode by NSC";
    static constexpr const char*    VENDOR_RISE         = "RiseRiseRise";
    static constexpr const char*    VENDOR_VORTEX       = "Vortex86 SoC";
    static constexpr const char*    VENDOR_VIA          = "VIA VIA VIA ";
    // Vendor-strings from Virtual Machines.
    static constexpr const char*    VENDOR_VMWARE       = "VMwareVMware";
    static constexpr const char*    VENDOR_XENHVM       = "XenVMMXenVMM";
    static constexpr const char*    VENDOR_MICROSOFT_HV = "Microsoft Hv";
    static constexpr const char*    VENDOR_PARALLELS    = " lrpepyh vr";

    static constexpr const uint32_t FEAT_ECX_SSE3       = 1 << 0;
    static constexpr const uint32_t FEAT_ECX_PCLMUL     = 1 << 1;
    static constexpr const uint32_t FEAT_ECX_DTES64     = 1 << 2;
    static constexpr const uint32_t FEAT_ECX_MONITOR    = 1 << 3;
    static constexpr const uint32_t FEAT_ECX_DS_CPL     = 1 << 4;
    static constexpr const uint32_t FEAT_ECX_VMX        = 1 << 5;
    static constexpr const uint32_t FEAT_ECX_SMX        = 1 << 6;
    static constexpr const uint32_t FEAT_ECX_EST        = 1 << 7;
    static constexpr const uint32_t FEAT_ECX_TM2        = 1 << 8;
    static constexpr const uint32_t FEAT_ECX_SSSE3      = 1 << 9;
    static constexpr const uint32_t FEAT_ECX_CID        = 1 << 10;
    static constexpr const uint32_t FEAT_ECX_FMA        = 1 << 12;
    static constexpr const uint32_t FEAT_ECX_CX16       = 1 << 13;
    static constexpr const uint32_t FEAT_ECX_ETPRD      = 1 << 14;
    static constexpr const uint32_t FEAT_ECX_PDCM       = 1 << 15;
    static constexpr const uint32_t FEAT_ECX_PCIDE      = 1 << 17;
    static constexpr const uint32_t FEAT_ECX_DCA        = 1 << 18;
    static constexpr const uint32_t FEAT_ECX_SSE4_1     = 1 << 19;
    static constexpr const uint32_t FEAT_ECX_SSE4_2     = 1 << 20;
    static constexpr const uint32_t FEAT_ECX_x2APIC     = 1 << 21;
    static constexpr const uint32_t FEAT_ECX_MOVBE      = 1 << 22;
    static constexpr const uint32_t FEAT_ECX_POPCNT     = 1 << 23;
    static constexpr const uint32_t FEAT_ECX_AES        = 1 << 25;
    static constexpr const uint32_t FEAT_ECX_XSAVE      = 1 << 26;
    static constexpr const uint32_t FEAT_ECX_OSXSAVE    = 1 << 27;
    static constexpr const uint32_t FEAT_ECX_AVX        = 1 << 28;
    static constexpr const uint32_t FEAT_EDX_FPU        = 1 << 0;
    static constexpr const uint32_t FEAT_EDX_VME        = 1 << 1;
    static constexpr const uint32_t FEAT_EDX_DE         = 1 << 2;
    static constexpr const uint32_t FEAT_EDX_PSE        = 1 << 3;
    static constexpr const uint32_t FEAT_EDX_TSC        = 1 << 4;
    static constexpr const uint32_t FEAT_EDX_MSR        = 1 << 5;
    static constexpr const uint32_t FEAT_EDX_PAE        = 1 << 6;
    static constexpr const uint32_t FEAT_EDX_MCE        = 1 << 7;
    static constexpr const uint32_t FEAT_EDX_CX8        = 1 << 8;
    static constexpr const uint32_t FEAT_EDX_APIC       = 1 << 9;
    static constexpr const uint32_t FEAT_EDX_SEP        = 1 << 11;
    static constexpr const uint32_t FEAT_EDX_MTRR       = 1 << 12;
    static constexpr const uint32_t FEAT_EDX_PGE        = 1 << 13;
    static constexpr const uint32_t FEAT_EDX_MCA        = 1 << 14;
    static constexpr const uint32_t FEAT_EDX_CMOV       = 1 << 15;
    static constexpr const uint32_t FEAT_EDX_PAT        = 1 << 16;
    static constexpr const uint32_t FEAT_EDX_PSE36      = 1 << 17;
    static constexpr const uint32_t FEAT_EDX_PSN        = 1 << 18;
    static constexpr const uint32_t FEAT_EDX_CLF        = 1 << 19;
    static constexpr const uint32_t FEAT_EDX_DTES       = 1 << 21;
    static constexpr const uint32_t FEAT_EDX_ACPI       = 1 << 22;
    static constexpr const uint32_t FEAT_EDX_MMX        = 1 << 23;
    static constexpr const uint32_t FEAT_EDX_FXSR       = 1 << 24;
    static constexpr const uint32_t FEAT_EDX_SSE        = 1 << 25;
    static constexpr const uint32_t FEAT_EDX_SSE2       = 1 << 26;
    static constexpr const uint32_t FEAT_EDX_SS         = 1 << 27;
    static constexpr const uint32_t FEAT_EDX_HTT        = 1 << 28;
    static constexpr const uint32_t FEAT_EDX_TM1        = 1 << 29;
    static constexpr const uint32_t FEAT_EDX_IA64       = 1 << 30;
    static constexpr const uint32_t FEAT_EDX_PBE        = 1 << 31;

    enum : uint32_t {
        GET_VENDOR = 0x00,
        GET_FEATURES,
        GET_TLB,
        GET_SERIAL,
        INTEL_EXTENDED = 0x80000000,
        INTEL_FEATURES,
        INTEL_BRANDSTRING,
        INTEL_BRANDSTRINGMORE,
        INTEL_BRANDSTRINGEND,
    };

    // TODO: 获取字符串信息
    char     vendor[VENDOR_LEN];
    char     brand[BRAND_LEN];
    char     family_name[FAMILY_LEN];
    char     model_name[MODEL_LEN];
    // 系列
    uint32_t family;
    // 型号
    uint32_t model;
    // 类型
    uint32_t type;
    // 频率
    uint32_t stepping;
    // cpu 基本信息
    uint32_t max_cpuid;
    // cpu 拓展信息
    uint32_t max_cpuidex;

    void     cpuid(uint32_t Mop, uint32_t Sop, uint32_t* eax, uint32_t* ebx,
                   uint32_t* ecx, uint32_t* edx) {
        __asm__ volatile("cpuid"
                         : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                         : "0"(Mop), "2"(Sop));
    }

public:
    CPUID(void) {
        uint32_t eax, ebx, ecx, edx;
        // 获取 vendor ID 与基本信息
        cpuid(GET_VENDOR, 0, &eax, &ebx, &ecx, &edx);
        max_cpuid = eax;
        memcpy(vendor, &ebx, 4);
        memcpy(vendor + 4, &edx, 4);
        memcpy(vendor + 8, &ecx, 4);
        vendor[12] = '\0';
        // 获取拓展信息
        cpuid(INTEL_EXTENDED, 0, &eax, &ebx, &ecx, &edx);
        max_cpuidex = eax;
        return;
    }

    ~CPUID(void) {
        return;
    }

    bool xapic(void) {
        uint32_t eax, ebx, ecx, edx;
        cpuid(GET_FEATURES, 0, &eax, &ebx, &ecx, &edx);
        return edx & FEAT_EDX_APIC;
    }

    bool x2apic(void) {
        uint32_t eax, ebx, ecx, edx;
        cpuid(GET_FEATURES, 0, &eax, &ebx, &ecx, &edx);
        return ecx & FEAT_ECX_x2APIC;
    }

    bool eoi(void) {
        uint64_t version = READ_MSR(IA32_X2APIC_VERSION);
        return version & IA32_X2APIC_SIVR_EOI_ENABLE_BIT;
    }
};

};     // namespace CPU

#endif /* SIMPLEKERNEL_CPU_HPP */
