
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

#ifndef _GDT_H_
#define _GDT_H_

#include "stdint.h"
#include "cpu.hpp"

/**
 * @brief GDT 接口
 * @see 64-ia-32-architectures-software-developer-vol-3a-manual Chapter3
 */
namespace GDT {
/// 全局描述符表长度
static constexpr const uint32_t GDT_LENGTH = 5;
/// 各个内存段所在全局描述符表下标
static constexpr const uint32_t GDT_NULL = 0;
/// 内核代码段
static constexpr const uint32_t GDT_KERNEL_CODE = 1;
/// 内核数据段
static constexpr const uint32_t GDT_KERNEL_DATA = 2;
/// 用户代码段
static constexpr const uint32_t GDT_USER_CODE = 3;
/// 用户数据段
static constexpr const uint32_t GDT_USER_DATA = 4;
/// 内核代码段选择子 0x08
static constexpr const uint32_t SEG_KERNEL_CODE = GDT_KERNEL_CODE << 3;
/// 内核数据段选择子
static constexpr const uint32_t SEG_KERNEL_DATA = GDT_KERNEL_DATA << 3;
/// 用户代码段选择子
static constexpr const uint32_t SEG_USER_CODE = GDT_USER_CODE << 3;
/// 用户数据段选择子
static constexpr const uint32_t SEG_USER_DATA = GDT_USER_DATA << 3;

// type 类型
// Code-and Data-Segment Types, S=1
/// @see 64-ia-32-architectures-software-developer-vol-3a-manual#3.4.5.1
/// 数据段 只读
static constexpr const uint32_t TYPE_DATA_READ_ONLY = 0x00;
/// 数据段 只读，可访问
static constexpr const uint32_t TYPE_DATA_READ_ONLY_ACCESSED = 0x01;
/// 数据段 读写
static constexpr const uint32_t TYPE_DATA_READ_WRITE = 0x02;
/// 数据段，读写，可访问
static constexpr const uint32_t TYPE_DATA_READ_WRITE_ACCESSED = 0x03;
/// 数据段，只读，？
static constexpr const uint32_t TYPE_DATA_READ_ONLY_EXPAND_DOWN = 0x04;
/// 数据段，只读，？，可访问
static constexpr const uint32_t TYPE_DATA_READ_ONLY_EXPAND_DOWN_ACCESSED = 0x05;
/// 数据段，读写，？
static constexpr const uint32_t TYPE_DATA_READ_WRITE_EXPAND_DOWN = 0x06;
/// 数据段，读写，？可访问
static constexpr const uint32_t TYPE_DATA_READ_WRITE_EXPAND_DOWN_ACCESSED =
    0x07;

/// 代码段，仅执行
static constexpr const uint32_t TYPE_CODE_EXECUTE_ONLY = 0x08;
/// 代码段，仅执行，可访问
static constexpr const uint32_t TYPE_CODE_EXECUTE_ONLY_ACCESSED = 0x09;
/// 代码段，可执行，可读
static constexpr const uint32_t TYPE_CODE_EXECUTE_READ = 0x0A;
/// 代码段，可执行，可读，可访问
static constexpr const uint32_t TYPE_CODE_EXECUTE_READ_ACCESSED = 0x0B;
/// 代码段，仅执行，？
static constexpr const uint32_t TYPE_CODE_EXECUTE_ONLY_CONFORMING = 0x0C;
/// 代码段，仅执行，？，可访问
static constexpr const uint32_t TYPE_CODE_EXECUTE_ONLY_CONFORMING_ACCESSED =
    0x0D;
/// 代码段，可执行，可读，？
static constexpr const uint32_t TYPE_CODE_EXECUTE_READ_CONFORMING = 0x0E;
/// 代码段，可执行，可读，？，可访问
static constexpr const uint32_t TYPE_CODE_EXECUTE_READ_CONFORMING_ACCESSED =
    0x0F;

// System-Segment and Gate-Descriptor Types, 32bit, when S=0
/// @see 64-ia-32-architectures-software-developer-vol-3a-manual#3.5
/// LDT
static constexpr const uint32_t TYPE_SYSTEM_LDT = 0x02;
// 64 位 TSS，可用
static constexpr const uint32_t TYPE_SYSTEM_64_TSS_AVAILABLE = 0x09;
// 64 位 TSS，在用
static constexpr const uint32_t TYPE_SYSTEM_64_TSS_BUSY = 0x0B;
// 64 位 调用门
static constexpr const uint32_t TYPE_SYSTEM_64_CALL_GATE = 0x0C;
// 64 位 中断门
static constexpr const uint32_t TYPE_SYSTEM_64_INTERRUPT_GATE = 0x0E;
// 64 位 陷阱门
static constexpr const uint32_t TYPE_SYSTEM_64_TRAP_GATE = 0x0F;

// S 位
/// 系统段
static constexpr const uint32_t S_SYSTEM = 0x00;
/// 代码/数据段
static constexpr const uint32_t S_CODE_DATA = 0x01;

// 各个段的全局描述符表的选择子
/// 内核代码段
static constexpr const uint32_t KERNEL_CS = SEG_KERNEL_CODE | CPU::DPL0;
/// 内核数据段
static constexpr const uint32_t KERNEL_DS = SEG_KERNEL_DATA | CPU::DPL0;
/// 用户代码段
static constexpr const uint32_t USER_CS = SEG_USER_CODE | CPU::DPL3;
/// 用户数据段
static constexpr const uint32_t USER_DS = SEG_USER_DATA | CPU::DPL3;

// P 位
/// 无效
static constexpr const uint32_t SEGMENT_NOT_PRESENT = 0x00;
/// 有效
static constexpr const uint32_t SEGMENT_PRESENT = 0x01;
// AVL
/// 无效
static constexpr const uint32_t AVL_NOT_AVAILABLE = 0x00;
/// 有效
static constexpr const uint32_t AVL_AVAILABLE = 0x01;
// L 位
/// 32 位
static constexpr const uint32_t L_32BIT = 0x00;
/// 64 位
static constexpr const uint32_t L_64BIT = 0x01;
// D/B
/// @todo
static constexpr const uint32_t DB_EXECUTABLE_CODE_SEGMENT_16 = 0x00;
/// @todo
static constexpr const uint32_t DB_EXECUTABLE_CODE_SEGMENT_32 = 0x01;
/// @todo
static constexpr const uint32_t DB_STACK_SEGMENT_STACK_POINTER_16 = 0x00;
/// @todo
static constexpr const uint32_t DB_STACK_SEGMENT_STACK_POINTER_32 = 0x01;
/// @todo
static constexpr const uint32_t DB_EXPAND_DOWN_DATA_SEGMENT_64KB = 0x00;
/// @todo
static constexpr const uint32_t DB_EXPAND_DOWN_DATA_SEGMENT_4GB = 0x01;
// G
/// 字节粒度
static constexpr const uint32_t G_BYTE = 0x00;
/// 4KB 粒度
static constexpr const uint32_t G_4KB = 0x01;

// 访问权限
/// 内核读，执行
static constexpr const uint32_t KREAD_EXEC = 0x9A;
/// 内核写
static constexpr const uint32_t KREAD_WRITE = 0x92;
/// 用户读，执行
static constexpr const uint32_t UREAD_EXEC = 0xFA;
/// 用户写
static constexpr const uint32_t UREAD_WRITE = 0xF2;

/// 段基址
static constexpr const uint32_t BASE = 0x00;
/// 段长度
static constexpr const uint32_t LIMIT = 0x00;

/**
 * @brief 全局描述符
 */
struct gdt_entry_t {
    /// 段界限 15:00，long 模式下忽略
    uint64_t limit1 : 16;
    /// 基址 15:00，long 模式下忽略
    uint64_t base_addr1 : 16;
    /// 基址 23:16，long 模式下忽略
    uint64_t base_addr2 : 8;
    /// 类型
    uint64_t type : 4;
    /// Descriptor type (0 = system; 1 = code or data)
    uint64_t s : 1;
    /// Specifies the privilege level of the segment
    uint64_t dpl : 2;
    /// Indicates whether the segment is present in memory (set) or not
    /// present (clear).
    uint64_t p : 1;
    /// 段界限 19:16，long 模式下忽略
    uint64_t limit2 : 4;
    /// Available for use by system software
    uint64_t avl : 1;
    /// 64-bit code segment (IA-32e mode only)
    uint64_t l : 1;
    /// Default operation size(0 = 16 - bit segment; 1 = 32 - bit segment)
    /// long 模式下忽略
    uint64_t db : 1;
    /// Determines the scaling of the segment limit field. When the
    /// granularity flag is clear, the segment limit is interpreted in
    /// byte units; when flag is set, the segment limit is interpreted in
    /// 4-KByte units.
    /// long 模式下忽略
    uint64_t g : 1;
    /// 基址 31:24，long 模式下忽略
    uint64_t base_addr3 : 8;
} __attribute__((packed));

/**
 * @brief 全剧描述符寄存器
 * @see 64-ia-32-architectures-software-developer-vol-3a-manual#3.5.1
 */
struct gdt_ptr64_t {
    // 全局描述符表限长
    uint16_t limit;
    // 全局描述符表 64位 基地址
    uint64_t base;
} __attribute__((packed));

/**
 * @brief 64 位 tss
 * @see 64-ia-32-architectures-software-developer-vol-3a-manual#7.7
 * @see 64-ia-32-architectures-software-developer-vol-3a-manual#7.2.3
 * @note 目前没有使用
 */
struct tss64_t {
    uint32_t reserved0;
    uint32_t rsp0_lower32;
    uint32_t rsp0_upper32;
    uint32_t rsp1_lower32;
    uint32_t rsp1_upper32;
    uint32_t rsp2_lower32;
    uint32_t rsp2_upper32;
    uint32_t reserved1;
    uint32_t reserved2;
    uint32_t ist1_lower32;
    uint32_t ist1_upper32;
    uint32_t ist2_lower32;
    uint32_t ist2_upper32;
    uint32_t ist3_lower32;
    uint32_t ist3_upper32;
    uint32_t ist4_lower32;
    uint32_t ist4_upper32;
    uint32_t ist5_lower32;
    uint32_t ist5_upper32;
    uint32_t ist6_lower32;
    uint32_t ist6_upper32;
    uint32_t ist7_lower32;
    uint32_t ist7_upper32;
    uint32_t reserved3;
    uint32_t reserved4;
    uint16_t reserved5;
    uint16_t io_map_base_addr;
} __attribute__((packed));

/**
 * @brief 全局描述符表构造函数
 * @param  _idx            描述符索引
 * @param  _base           基址
 * @param  _limit          长度
 * @param  _type           @todo
 * @param  _s              @todo
 * @param  _dpl            权限
 * @param  _p              有效
 * @param  _avl            @todo
 * @param  _l              32/64位
 * @param  _db             @todo
 * @param  _g              粒度
 */
void set_gdt(uint8_t _idx, uint32_t _base, uint32_t _limit, uint8_t _type,
             uint8_t _s, uint8_t _dpl, uint8_t _p, uint8_t _avl, uint8_t _l,
             uint8_t _db, uint8_t _g);

/**
 * @brief 初始化
 * @return int32_t         成功返回 0
 * @todo 与 32 位合并
 * @todo 精简代码
 */
int32_t init(void);
}; // namespace GDT

#endif /* _GDT_H_ */
