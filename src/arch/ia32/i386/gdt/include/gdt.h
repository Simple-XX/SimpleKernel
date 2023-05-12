
/**
 * @file gdt.h
 * @brief 描述符抽象头文件
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

#ifndef SIMPLEKERNEL_GDT_H
#define SIMPLEKERNEL_GDT_H

#include "cpu.hpp"
#include "cstdint"

/**
 * @brief GDT 接口
 * @see 64-ia-32-architectures-software-developer-vol-3a-manual Chapter3
 */
namespace GDT {
/// 全局描述符表长度
static constexpr const uint32_t GDT_LENGTH          = 5;
/// 各个内存段所在全局描述符表下标
static constexpr const uint32_t GDT_NULL            = 0;
/// 内核代码段
static constexpr const uint32_t GDT_KERNEL_CODE     = 1;
/// 内核数据段
static constexpr const uint32_t GDT_KERNEL_DATA     = 2;
/// 用户代码段
static constexpr const uint32_t GDT_USER_CODE       = 3;
/// 用户数据段
static constexpr const uint32_t GDT_USER_DATA       = 4;
/// 内核代码段选择子 0x08
static constexpr const uint32_t SEG_KERNEL_CODE     = GDT_KERNEL_CODE << 3;
/// 内核数据段选择子
static constexpr const uint32_t SEG_KERNEL_DATA     = GDT_KERNEL_DATA << 3;
/// 用户代码段选择子
static constexpr const uint32_t SEG_USER_CODE       = GDT_USER_CODE << 3;
/// 用户数据段选择子
static constexpr const uint32_t SEG_USER_DATA       = GDT_USER_DATA << 3;

// type 类型
// Code- and Data-Segment Types, S=1
/// 数据段 只读
static constexpr const uint32_t TYPE_DATA_READ_ONLY = 0x00;
/// 数据段 只读，可访问
static constexpr const uint32_t TYPE_DATA_READ_ONLY_ACCESSED             = 0x01;
/// 数据段 读写
static constexpr const uint32_t TYPE_DATA_READ_WRITE                     = 0x02;
/// 数据段，读写，可访问
static constexpr const uint32_t TYPE_DATA_READ_WRITE_ACCESSED            = 0x03;
/// 数据段，只读，？
static constexpr const uint32_t TYPE_DATA_READ_ONLY_EXPAND_DOWN          = 0x04;
/// 数据段，只读，？，可访问
static constexpr const uint32_t TYPE_DATA_READ_ONLY_EXPAND_DOWN_ACCESSED = 0x05;
/// 数据段，读写，？
static constexpr const uint32_t TYPE_DATA_READ_WRITE_EXPAND_DOWN         = 0x06;
/// 数据段，读写，？可访问
static constexpr const uint32_t TYPE_DATA_READ_WRITE_EXPAND_DOWN_ACCESSED
  = 0x07;
/// 代码段，仅执行
static constexpr const uint32_t TYPE_CODE_EXECUTE_ONLY            = 0x08;
/// 代码段，仅执行，可访问
static constexpr const uint32_t TYPE_CODE_EXECUTE_ONLY_ACCESSED   = 0x09;
/// 代码段，可执行，可读
static constexpr const uint32_t TYPE_CODE_EXECUTE_READ            = 0x0A;
/// 代码段，可执行，可读，可访问
static constexpr const uint32_t TYPE_CODE_EXECUTE_READ_ACCESSED   = 0x0B;
/// 代码段，仅执行，？
static constexpr const uint32_t TYPE_CODE_EXECUTE_ONLY_CONFORMING = 0x0C;
/// 代码段，仅执行，？，可访问
static constexpr const uint32_t TYPE_CODE_EXECUTE_ONLY_CONFORMING_ACCESSED
  = 0x0D;
/// 代码段，可执行，可读，？
static constexpr const uint32_t TYPE_CODE_EXECUTE_READ_CONFORMING = 0x0E;
/// 代码段，可执行，可读，？，可访问
static constexpr const uint32_t TYPE_CODE_EXECUTE_READ_CONFORMING_ACCESSED
  = 0x0F;

// System-Segment and Gate-Descriptor Types, 32bit, when S=0
/// 保留
static constexpr const uint32_t TYPE_SYSTEM_16_RESERVESD1     = 0x00;
/// 16 位 TSS
static constexpr const uint32_t TYPE_SYSTEM_16_TSS_AVAILABLE  = 0x01;
/// LDT
static constexpr const uint32_t TYPE_SYSTEM_LDT               = 0x02;
/// 16 位 TSS，在用
static constexpr const uint32_t TYPE_SYSTEM_16_TSS_BUSY       = 0x03;
/// 16 位调用门
static constexpr const uint32_t TYPE_SYSTEM_16_CALL_GATE      = 0x04;
/// 任务门
static constexpr const uint32_t TYPE_SYSTEM_TASK_GATE         = 0x05;
/// 16 位 中断门
static constexpr const uint32_t TYPE_SYSTEM_16_INTERRUPT_GATE = 0x06;
/// 16 位 陷阱门
static constexpr const uint32_t TYPE_SYSTEM_16_TRAP_GATE      = 0x07;
/// 保留
static constexpr const uint32_t TYPE_SYSTEM_32_RESERVESD2     = 0x08;
/// 32 位 TSS
static constexpr const uint32_t TYPE_SYSTEM_32_TSS_AVAILABLE  = 0x09;
/// 保留
static constexpr const uint32_t TYPE_SYSTEM_32_RESERVESD3     = 0x0A;
/// 32 位，在用
static constexpr const uint32_t TYPE_SYSTEM_32_TSS_BUSY       = 0x0B;
/// 32 位 调用门
static constexpr const uint32_t TYPE_SYSTEM_32_CALL_GATE      = 0x0C;
/// 保留
static constexpr const uint32_t TYPE_SYSTEM_RESERVESD4        = 0x0D;
/// 32 位 中断门
static constexpr const uint32_t TYPE_SYSTEM_32_INTERRUPT_GATE = 0x0E;
/// 32 位 陷阱门
static constexpr const uint32_t TYPE_SYSTEM_32_TRAP_GATE      = 0x0F;

// S 位
/// 系统段
static constexpr const uint32_t S_SYSTEM                      = 0x00;
/// 代码/数据段
static constexpr const uint32_t S_CODE_DATA                   = 0x01;

// 各个段的全局描述符表的选择子
/// 内核代码段
static constexpr const uint32_t KERNEL_CS = SEG_KERNEL_CODE | CPU::DPL0;
/// 内核数据段
static constexpr const uint32_t KERNEL_DS = SEG_KERNEL_DATA | CPU::DPL0;
/// 用户代码段
static constexpr const uint32_t USER_CS   = SEG_USER_CODE | CPU::DPL3;
/// 用户数据段
static constexpr const uint32_t USER_DS   = SEG_USER_DATA | CPU::DPL3;

// P 位
/// 无效
static constexpr const uint32_t SEGMENT_NOT_PRESENT               = 0x00;
/// 有效
static constexpr const uint32_t SEGMENT_PRESENT                   = 0x01;
// AVL
/// 无效
static constexpr const uint32_t AVL_NOT_AVAILABLE                 = 0x00;
/// 有效
static constexpr const uint32_t AVL_AVAILABLE                     = 0x01;
// L 位
/// 32 位
static constexpr const uint32_t L_32BIT                           = 0x00;
/// 64 位
static constexpr const uint32_t L_64BIT                           = 0x01;
// D/B
/// @todo
static constexpr const uint32_t DB_EXECUTABLE_CODE_SEGMENT_16     = 0x00;
/// @todo
static constexpr const uint32_t DB_EXECUTABLE_CODE_SEGMENT_32     = 0x01;
/// @todo
static constexpr const uint32_t DB_STACK_SEGMENT_STACK_POINTER_16 = 0x00;
/// @todo
static constexpr const uint32_t DB_STACK_SEGMENT_STACK_POINTER_32 = 0x01;
/// @todo
static constexpr const uint32_t DB_EXPAND_DOWN_DATA_SEGMENT_64KB  = 0x00;
/// @todo
static constexpr const uint32_t DB_EXPAND_DOWN_DATA_SEGMENT_4GB   = 0x01;
// G
/// 字节粒度
static constexpr const uint32_t G_BYTE                            = 0x00;
/// 4KB 粒度
static constexpr const uint32_t G_4KB                             = 0x01;

// 访问权限
/// 内核读，执行
static constexpr const uint32_t KREAD_EXEC                        = 0x9A;
/// 内核写
static constexpr const uint32_t KREAD_WRITE                       = 0x92;
/// 用户读，执行
static constexpr const uint32_t UREAD_EXEC                        = 0xFA;
/// 用户写
static constexpr const uint32_t UREAD_WRITE                       = 0xF2;

/// 段基址
static constexpr const uint32_t BASE                              = 0x00;
/// 段长度
static constexpr const uint32_t LIMIT                             = 0xFFFFFFFF;

/**
 * @brief 全局描述符
 */
struct gdt_entry_t {
    /// 段界限 15:00
    uint64_t limit1     : 16;
    /// 基址 15:00
    uint64_t base_addr1 : 16;
    /// 基址 23:16
    uint64_t base_addr2 : 8;
    /// 类型
    uint64_t type       : 4;
    /// Descriptor type (0 = system; 1 = code or data)
    uint64_t s          : 1;
    /// Specifies the privilege level of the segment
    uint64_t dpl        : 2;
    /// Indicates whether the segment is present in memory (set) or not
    /// present (clear).
    uint64_t p          : 1;
    /// 段界限 19:16
    uint64_t limit2     : 4;
    /// Available for use by system software
    uint64_t avl        : 1;
    /// 64-bit code segment (IA-32e mode only)
    uint64_t l          : 1;
    /// Default operation size(0 = 16 - bit segment; 1 = 32 - bit segment)
    uint64_t db         : 1;
    /// Determines the scaling of the segment limit field. When the
    /// granularity flag is clear, the segment limit is interpreted in
    /// byte units; when flag is set, the segment limit is interpreted in
    /// 4-KByte units.
    uint64_t g          : 1;
    /// 基址 31:24
    uint64_t base_addr3 : 8;
} __attribute__((packed));

/**
 * @brief 全剧描述符寄存器
 */
struct gdt_ptr_t {
    /// 全局描述符表限长
    uint16_t limit;
    /// 全局描述符表 32位 基地址
    uint32_t base;
} __attribute__((packed));

/**
 * @brief 32 位 tss
 * @see 64-ia-32-architectures-software-developer-vol-3a-manual#7.2.1
 * @note 目前没有使用
 */
struct tss32_t {
    uint16_t prev_task_link;
    uint16_t reserved0;
    uint32_t esp0;
    uint16_t ss0;
    uint32_t reserved1;
    uint32_t esp1;
    uint16_t ss1;
    uint32_t reserved2;
    uint32_t esp2;
    uint16_t ss2;
    uint32_t reserved3;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint16_t es;
    uint16_t reserved4;
    uint16_t cs;
    uint16_t reserved5;
    uint16_t ss;
    uint16_t reserved6;
    uint16_t ds;
    uint16_t reserved7;
    uint16_t fs;
    uint16_t reserved8;
    uint16_t gs;
    uint16_t reserved9;
    uint16_t ldt_segment_selector;
    uint16_t reserved10;
    uint32_t t                : 1;
    uint32_t reserved11       : 15;
    uint32_t io_map_base_addr : 16;
} __attribute__((packed));

/**
 * @brief 全局描述符表构造函数
 * @param  _num             描述符索引
 * @param  _base            基地
 * @param  _limit           长度
 * @param  _access          访问权限
 * @param  _gran            粒度
 */
void    set_gdt(int32_t _num, uint32_t _base, uint32_t _limit, uint8_t _access,
                uint8_t _gran);

/**
 * @brief 初始化
 * @return int32_t         成功返回 0
 */
int32_t init(void);
};     // namespace GDT

#endif /* SIMPLEKERNEL_GDT_H */
