
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// port.h for MRNIU/SimpleKernel.


#ifndef _CPU_HPP_
#define _CPU_HPP_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdbool.h"

#define EFLAGS_CF           0x00000001  // Carry Flag
#define EFLAGS_PF           0x00000004  // Parity Flag
#define EFLAGS_AF           0x00000010  // Auxiliary carry Flag
#define EFLAGS_ZF           0x00000040  // Zero Flag
#define EFLAGS_SF           0x00000080  // Sign Flag
#define EFLAGS_TF           0x00000100  // Trap Flag
#define EFLAGS_IF           0x00000200  // Interrupt Flag
#define EFLAGS_DF           0x00000400  // Direction Flag
#define EFLAGS_OF           0x00000800  // Overflow Flag
#define EFLAGS_IOPL_MASK    0x00003000  // I/O Privilege Level bitmask
#define EFLAGS_IOPL_0       0x00000000  // IOPL == 0
#define EFLAGS_IOPL_1       0x00001000  // IOPL == 1
#define EFLAGS_IOPL_2       0x00002000  // IOPL == 2
#define EFLAGS_IOPL_3       0x00003000  // IOPL == 3
#define EFLAGS_NT           0x00004000  // Nested Task
#define EFLAGS_RF           0x00010000  // Resume Flag
#define EFLAGS_VM           0x00020000  // Virtual 8086 mode
#define EFLAGS_AC           0x00040000  // Alignment Check
#define EFLAGS_VIF          0x00080000  // Virtual Interrupt Flag
#define EFLAGS_VIP          0x00100000  // Virtual Interrupt Pending
#define EFLAGS_ID           0x00200000  // ID flag

// CR0：包含当前处理器运行的控制标志。
// CR1：保留。
// CR2：包含发生页面错误时的线性地址。
// CR3：页面目录表（Page Directory Table）的物理地址。
// 虚拟地址启用且CR0中PG位设置为1的情况下，CR3可以协助处理器将线性地址转换为物理地址。一般情况下为MMU提供页表的入口实现。
// CR4：包含处理器扩展功能的标志位。
// CR8：提供对任务优先级寄存器（Task Priority Register）的读写（仅在64位模式下存在）。
// 对控制寄存器的读写是通过MOV CRn指令来实现

// PE：CR0的位0是启用保护（Protection Enable）标志。当设置该位时即开启了保护模式；
// 当复位时即进入实地址模式。这个标志仅开启段级保护，而并没有启用分页机制。若要启用分页机制，那么PE和PG标志都要置位。
#define CR0_PE       0x00000001
#define CR0_MP       0x00000002
#define CR0_EM       0x00000004
#define CR0_TS       0x00000008
#define CR0_ET       0x00000010

// NE：对于 Intel 80486或以上的CPU，CR0 的位5是协处理器错误（Numeric Error）标志。
// 当设置该标志时，就启用了 x87 协处理器错误的内部报告机制；若复位该标志，那么就使用 PC 形式的 x87 协处理器错误报告机制。
// 当NE为复位状态并且 CPU 的 IGNNE 输入引脚有信号时，那么数学协处理器 x87 错误将被忽略。
// 当NE为复位状态并且 CPU 的 IGNNE 输入引脚无信号时，那么非屏蔽的数学协处理器 x87 错误将导致处理器通过 FERR 引脚在外部产生一个中断，
// 并且在执行下一个等待形式浮点指令或 WAIT/FWAIT 指令之前立刻停止指令执行。 CPU 的 FERR 引脚用于仿真外部协处理器 80387 的 ERROR 引脚，
// 因此通常连接到中断控制器输入请求引脚上。NE 标志、IGNNE 引脚和 FERR 引脚用于利用外部逻辑来实现 PC 形式的外部错误报告机制。
#define CR0_NE       0x00000020

// WP：对于Intel 80486或以上的CPU，CR0的位16是写保护（Write Proctect）标志。
// 当设置该标志时，处理器会禁止超级用户程序（例如特权级0的程序）向用户级只读页面执行写操作；当该位复位时则反之。该标志有利于UNIX类操作系统在创建进程时实现写时复制（Copy on Write）技术。
#define CR0_WP       0x00010020
#define CR0_AM       0x00040020
#define CR0_NW       0x20000000
#define CR0_CD       0x40000000

// PG：CR0的位31是分页（Paging）标志。当设置该位时即开启了分页机制；
// 当复位时则禁止分页机制，此时所有线性地址等同于物理地址。在开启这个标志之前必须已经或者同时开启PE标志。即若要启用分页机制，那么PE和PG标志都要置位。
#define CR0_PG       0x80000000

// 启用保护模式PE（Protected Enable）位（位0）和开启分页PG（Paging）位（位31）分别用于控制分段和分页机制。
// PE用于控制分段机制。如果PE=1，处理器就工作在开启分段机制环境下，即运行在保护模式下。
// 如果PE=0，则处理器关闭了分段机制，并如同8086工作于实地址模式下。PG用于控制分页机制。如果PG=1，则开启了分页机制。
// 如果PG=0，分页机制被禁止，此时线性地址被直接作为物理地址使用。
// 如果PE=0、PG=0，处理器工作在实地址模式下；如果PG=0、PE=1，处理器工作在没有开启分页机制的保护模式下；
// 如果PG=1、PE=0，此时由于不在保护模式下不能启用分页机制，因此处理器会产生一个一般保护异常，即这种标志组合无效；
// 如果PG=1、PE=1，则处理器工作在开启了分页机制的保护模式下。


#define CR3_PWT     0x00000008
#define CR3_PCD     0x00000010

#define CR4_VME        0x00000001
#define CR4_PVI        0x00000002
#define CR4_TSD        0x00000004
#define CR4_DE         0x00000008
#define CR4_PSE        0x00000010
#define CR4_PAE        0x00000020
#define CR4_MCE        0x00000040
#define CR4_PGE        0x00000080
#define CR4_PCE        0x00000100
#define CR4_OSFXSR     0x00000200
#define CR4_OSXMMEXCPT 0x00000400
#define CR4_VMXE       0x00002000
#define CR4_SMXE       0x00004000
#define CR4_PCIDE      0x00020000
#define CR4_OSXSAVE    0x00040000
#define CR4_SMEP       0x00100000

// 执行CPU空操作
static inline void cpu_hlt(void) {
	asm volatile ( "hlt" );
}

// 开启中断
static inline void cpu_sti(void) {
	asm volatile ( "sti" );
}

// 关闭中断
static inline void cpu_cli(void) {
	asm volatile ( "cli" ::: "memory" );
}

// 读取 EFLAGS
static inline uint64_t read_eflags(void) {
	uint64_t eflags;
	asm volatile ( "pushf;pop %0"
	               : "=r" ( eflags ) );
	return eflags;
}

// 读取 EFLAGS
static inline void debug_intr(void) {
	__asm__ __volatile__ ( "int $0x01" );
	return;
}


// Identification flag
//程序能够设置或清除这个标志指示了处理器对 CPUID 指令的支持。
static inline bool FL_ID_status(void) {
	uint64_t eflags = read_eflags();
	return ( eflags & EFLAGS_ID );
}

// Virtual interrupt pending flag
// 该位置1以指示一个中断正在被挂起，当没有中断挂起时该位清零。
// 与VIF标志结合使用。

static inline bool EFLAGS_VIP_status(void) {
	uint32_t eflags = read_eflags();
	return ( eflags & EFLAGS_VIP );
}

// Virtual interrupt flag
// 该标志是IF标志的虚拟镜像(Virtual image)，与VIP标志结合起来使用。
// 使用这个标志以及VIP标志，并设置CR4控制寄存器中的VME标志就可以允许虚拟模式扩展(virtual mode extensions)
static inline bool EFLAGS_VIF_status(void) {
	uint32_t eflags = read_eflags();
	return ( eflags & EFLAGS_VIF );
}

// Alignment check flag 地址中的对齐检查
static inline bool EFLAGS_AC_status(void) {
	uint32_t eflags = read_eflags();
	return ( eflags & EFLAGS_AC );
}

// 虚拟 8086 ，为 1 时进入
static inline bool EFLAGS_VM_status(void) {
	uint32_t eflags = read_eflags();
	return ( eflags & EFLAGS_VM );
}

// Resume flag 控制处理器对调试异常的响应。
static inline bool EFLAGS_RF_status(void) {
	uint32_t eflags = read_eflags();
	return ( eflags & EFLAGS_RF );
}

// Nested task flag
// 这个标志控制中断链和被调用任务。若当前任务与前一个执行任务相关则置 1，反之则清零。
static inline bool EFLAGS_NT_status(void) {
	uint32_t eflags = read_eflags();
	return ( eflags & EFLAGS_NT );
}

// 权限标志
static inline uint32_t get_IOPL(void) {
	uint32_t eflags = read_eflags();
	uint32_t level = 0;
	if (eflags & EFLAGS_IOPL_0)
		level = 0;
	else if (eflags & EFLAGS_IOPL_1)
		level = 1;
	else if (eflags & EFLAGS_IOPL_2)
		level = 2;
	else if (eflags & EFLAGS_IOPL_3)
		level = 3;
	else return 2333;
	return level;
}

// 溢出标志
static inline bool EFLAGS_OF_status(void) {
	uint32_t eflags = read_eflags();
	return ( eflags & EFLAGS_OF );
}

// 控制串指令(MOVS, CMPS, SCAS, LODS以及STOS)。
// 设置DF标志使得串指令自动递减（从高地址向低地址方向处理字符串）
// 清除该标志则使得串指令自动递增。
// STD以及CLD指令分别用于设置以及清除DF标志。
static inline bool EFLAGS_DF_status(void) {
	uint32_t eflags = read_eflags();
	return ( eflags & EFLAGS_DF );
}

// 中断标志
static inline bool EFLAGS_IF_status(void) {
	uint32_t eflags = read_eflags();
	return ( eflags & EFLAGS_IF );
}

static inline bool EFLAGS_TF_status(void) {
	uint32_t eflags = read_eflags();
	return ( eflags & EFLAGS_TF );
}

// Sign flag 符号标志
static inline bool EFLAGS_SF_status(void) {
	uint32_t eflags = read_eflags();
	return ( eflags & EFLAGS_SF );
}

// Zero flag 零标志
static inline bool EFLAGS_ZF_status(void) {
	uint32_t eflags = read_eflags();
	return ( eflags & EFLAGS_ZF );
}

// Adjust flag调整位
static inline bool EFLAGS_AF_status(void) {
	uint32_t eflags = read_eflags();
	return ( eflags & EFLAGS_AF );
}

// Parity flag奇偶位
static inline bool EFLAGS_PF_status(void) {
	uint32_t eflags = read_eflags();
	return ( eflags & EFLAGS_PF );
}

// Carry flag 进位标志
static inline bool EFLAGS_CF_status(void) {
	uint32_t eflags = read_eflags();
	return ( eflags & EFLAGS_CF );
}

static inline void __native_flush_tlb_single(unsigned long addr) {
	asm volatile ( "invlpg (%0)" : : "r" ( addr ) : "memory" );
	return;
}

#ifdef __cplusplus
}
#endif

#endif /* _CPU_HPP_ */
