
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// gdt.h for Simple-XX/SimpleKernel.

#ifndef _GDT_H_
#define _GDT_H_

#include "stdint.h"
#include "cpu.hpp"

// See 64-ia-32-architectures-software-developer-vol-3a-manual Chapter3

namespace GDT {
    // 全局描述符表长度
    static constexpr const uint32_t GDT_LENGTH = 5;
    // 各个内存段所在全局描述符表下标
    static constexpr const uint32_t GDT_NULL        = 0;
    static constexpr const uint32_t GDT_KERNEL_CODE = 1;
    static constexpr const uint32_t GDT_KERNEL_DATA = 2;
    static constexpr const uint32_t GDT_USER_CODE   = 3;
    static constexpr const uint32_t GDT_USER_DATA   = 4;
    // 内核代码段 0x08
    static constexpr const uint32_t SEG_KERNEL_CODE = GDT_KERNEL_CODE << 3;
    // 内核数据段
    static constexpr const uint32_t SEG_KERNEL_DATA = GDT_KERNEL_DATA << 3;
    // 用户代码段
    static constexpr const uint32_t SEG_USER_CODE = GDT_USER_CODE << 3;
    // 用户数据段
    static constexpr const uint32_t SEG_USER_DATA = GDT_USER_DATA << 3;

    // type 类型
    // Code- and Data-Segment Types, S=1
    static constexpr const uint32_t TYPE_DATA_READ_ONLY             = 0x00;
    static constexpr const uint32_t TYPE_DATA_READ_ONLY_ACCESSED    = 0x01;
    static constexpr const uint32_t TYPE_DATA_READ_WRITE            = 0x02;
    static constexpr const uint32_t TYPE_DATA_READ_WRITE_ACCESSED   = 0x03;
    static constexpr const uint32_t TYPE_DATA_READ_ONLY_EXPAND_DOWN = 0x04;
    static constexpr const uint32_t TYPE_DATA_READ_ONLY_EXPAND_DOWN_ACCESSED =
        0x05;
    static constexpr const uint32_t TYPE_DATA_READ_WRITE_EXPAND_DOWN = 0x06;
    static constexpr const uint32_t TYPE_DATA_READ_WRITE_EXPAND_DOWN_ACCESSED =
        0x07;

    static constexpr const uint32_t TYPE_CODE_EXECUTE_ONLY            = 0x08;
    static constexpr const uint32_t TYPE_CODE_EXECUTE_ONLY_ACCESSED   = 0x09;
    static constexpr const uint32_t TYPE_CODE_EXECUTE_READ            = 0x0A;
    static constexpr const uint32_t TYPE_CODE_EXECUTE_READ_ACCESSED   = 0x0B;
    static constexpr const uint32_t TYPE_CODE_EXECUTE_ONLY_CONFORMING = 0x0C;
    static constexpr const uint32_t TYPE_CODE_EXECUTE_ONLY_CONFORMING_ACCESSED =
        0x0D;
    static constexpr const uint32_t TYPE_CODE_EXECUTE_READ_CONFORMING = 0x0E;
    static constexpr const uint32_t TYPE_CODE_EXECUTE_READ_CONFORMING_ACCESSED =
        0x0F;

    // System-Segment and Gate-Descriptor Types, 32bit, when S=0
    static constexpr const uint32_t TYPE_SYSTEM_16_RESERVESD1     = 0x00;
    static constexpr const uint32_t TYPE_SYSTEM_16_TSS_AVAILABLE  = 0x01;
    static constexpr const uint32_t TYPE_SYSTEM_LDT               = 0x02;
    static constexpr const uint32_t TYPE_SYSTEM_16_TSS_BUSY       = 0x03;
    static constexpr const uint32_t TYPE_SYSTEM_16_CALL_GATE      = 0x04;
    static constexpr const uint32_t TYPE_SYSTEM_TASK_GATE         = 0x05;
    static constexpr const uint32_t TYPE_SYSTEM_16_INTERRUPT_GATE = 0x06;
    static constexpr const uint32_t TYPE_SYSTEM_16_TRAP_GATE      = 0x07;
    static constexpr const uint32_t TYPE_SYSTEM_32_RESERVESD2     = 0x08;
    static constexpr const uint32_t TYPE_SYSTEM_32_TSS_AVAILABLE  = 0x09;
    static constexpr const uint32_t TYPE_SYSTEM_32_RESERVESD3     = 0x0A;
    static constexpr const uint32_t TYPE_SYSTEM_32_TSS_BUSY       = 0x0B;
    static constexpr const uint32_t TYPE_SYSTEM_32_CALL_GATE      = 0x0C;
    static constexpr const uint32_t TYPE_SYSTEM_RESERVESD4        = 0x0D;
    static constexpr const uint32_t TYPE_SYSTEM_32_INTERRUPT_GATE = 0x0E;
    static constexpr const uint32_t TYPE_SYSTEM_32_TRAP_GATE      = 0x0F;

    // S 位
    static constexpr const uint32_t S_SYSTEM    = 0x00;
    static constexpr const uint32_t S_CODE_DATA = 0x01;

    // 各个段的全局描述符表的选择子
    static constexpr const uint32_t KERNEL_CS = SEG_KERNEL_CODE | CPU::DPL0;
    static constexpr const uint32_t KERNEL_DS = SEG_KERNEL_DATA | CPU::DPL0;
    static constexpr const uint32_t USER_CS   = SEG_USER_CODE | CPU::DPL3;
    static constexpr const uint32_t USER_DS   = SEG_USER_DATA | CPU::DPL3;

    // P 位
    static constexpr const uint32_t SEGMENT_NOT_PRESENT = 0x00;
    static constexpr const uint32_t SEGMENT_PRESENT     = 0x01;
    // AVL
    static constexpr const uint32_t AVL_NOT_AVAILABLE = 0x00;
    static constexpr const uint32_t AVL_AVAILABLE     = 0x01;
    // L 位
    static constexpr const uint32_t L_32BIT = 0x00;
    static constexpr const uint32_t L_64BIT = 0x01;
    // D/B
    static constexpr const uint32_t DB_EXECUTABLE_CODE_SEGMENT_16     = 0x00;
    static constexpr const uint32_t DB_EXECUTABLE_CODE_SEGMENT_32     = 0x01;
    static constexpr const uint32_t DB_STACK_SEGMENT_STACK_POINTER_16 = 0x00;
    static constexpr const uint32_t DB_STACK_SEGMENT_STACK_POINTER_32 = 0x01;
    static constexpr const uint32_t DB_EXPAND_DOWN_DATA_SEGMENT_64KB  = 0x00;
    static constexpr const uint32_t DB_EXPAND_DOWN_DATA_SEGMENT_4GB   = 0x01;
    // G
    static constexpr const uint32_t G_BYTE = 0x00;
    static constexpr const uint32_t G_4KB  = 0x01;

    // 访问权限
    static constexpr const uint32_t KREAD_EXEC  = 0x9A;
    static constexpr const uint32_t KREAD_WRITE = 0x92;
    static constexpr const uint32_t UREAD_EXEC  = 0xFA;
    static constexpr const uint32_t UREAD_WRITE = 0xF2;

    static constexpr const uint32_t BASE  = 0x00;
    static constexpr const uint32_t LIMIT = 0xFFFFFFFF;

    // 全局描述符类型
    struct gdt_entry_t {
        // 段界限 15:00
        uint64_t limit1 : 16;
        // 基址 15:00
        uint64_t base_addr1 : 16;
        // 基址 23:16
        uint64_t base_addr2 : 8;
        // 类型
        uint64_t type : 4;
        // Descriptor type (0 = system; 1 = code or data)
        uint64_t s : 1;
        // Specifies the privilege level of the segment
        uint64_t dpl : 2;
        // Indicates whether the segment is present in memory (set) or not
        // present (clear).
        uint64_t p : 1;
        // 段界限 19:16
        uint64_t limit2 : 4;
        // Available for use by system software
        uint64_t avl : 1;
        // 64-bit code segment (IA-32e mode only)
        uint64_t l : 1;
        // Default operation size(0 = 16 - bit segment; 1 = 32 - bit segment)
        uint64_t db : 1;
        // Determines the scaling of the segment limit field. When the
        // granularity flag is clear, the segment limit is interpreted in
        // byte units; when flag is set, the segment limit is interpreted in
        // 4-KByte units.
        uint64_t g : 1;
        // 基址 31:24
        uint64_t base_addr3 : 8;
    } __attribute__((packed));

    // GDTR
    struct gdt_ptr_t {
        // 全局描述符表限长
        uint16_t limit;
        // 全局描述符表 32位 基地址
        uint32_t base;
    } __attribute__((packed));

    // 64-ia-32-architectures-software-developer-vol-3a-manual#7.2.1
    // 目前没有使用
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
        uint32_t t : 1;
        uint32_t reserved11 : 15;
        uint32_t io_map_base_addr : 16;
    } __attribute__((packed));
    // 全局描述符表构造函数，根据下标构造
    // 参数:
    // num-数组下标、base-基地址、limit-限长、access-访问标志，gran-粒度
    void set_gdt(int32_t num, uint32_t base, uint32_t limit, uint8_t access,
                 uint8_t gran);
    // 初始化
    int32_t init(void);
};

#endif /* _GDT_H_ */
