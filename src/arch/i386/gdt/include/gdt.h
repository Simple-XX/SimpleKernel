
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
    static constexpr const uint32_t GDT_LENGTH = 6;
    // 各个内存段所在全局描述符表下标
    static constexpr const uint32_t GDT_NULL        = 0;
    static constexpr const uint32_t GDT_KERNEL_CODE = 1;
    static constexpr const uint32_t GDT_KERNEL_DATA = 2;
    static constexpr const uint32_t GDT_USER_CODE   = 3;
    static constexpr const uint32_t GDT_USER_DATA   = 4;
    static constexpr const uint32_t GDT_TSS         = 5;
    // 内核代码段 0x08
    static constexpr const uint32_t SEG_KERNEL_CODE = GDT_KERNEL_CODE << 3;
    // 内核数据段
    static constexpr const uint32_t SEG_KERNEL_DATA = GDT_KERNEL_DATA << 3;
    // 用户代码段
    static constexpr const uint32_t SEG_USER_CODE = GDT_USER_CODE << 3;
    // 用户数据段
    static constexpr const uint32_t SEG_USER_DATA = GDT_USER_DATA << 3;
    // 任务段
    static constexpr const uint32_t SEG_TSS = GDT_TSS << 3;

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
    typedef struct gdt_entry {
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
    } __attribute__((packed)) gdt_entry_t;

    // GDTR
    typedef struct gdt_ptr {
        // 全局描述符表限长
        uint16_t limit;
        // 全局描述符表 32位 基地址
        uint32_t base;
    } __attribute__((packed)) gdt_ptr_t;

    // TSS 状态段由两部分组成：
    // 1. 动态部分(处理器在每次任务切换时会设置这些字段值)
    //    通用寄存器(EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI)
    //    段寄存器(ES，CS，SS，DS，FS，GS)
    //    状态寄存器(EFLAGS)
    //    指令指针(EIP)
    //    前一个执行的任务的TSS段的选择子(只有当要返回时才更新)
    // 2. 静态字段(处理器读取，但从不更改)
    //    任务的LDT选择子
    //    页目录基址寄存器(PDBR)(当启用分页时，只读)
    //    内层堆栈指针，特权级 0-2
    //    T-位，指示了处理器在任务切换时是否引发一个调试异常
    //    I/O 位图基址

    // TSS(任务状态段) 描述符
    // TSS的使用是为了解决调用门中特权级变换时堆栈发生的变化.
    // 资料：intel 手册 3ACh7
    typedef struct tss_struct {
        // old ts selector
        uint32_t ts_link;
        // stack pointers and segment selectors
        uint32_t ts_esp0;
        // after an increase in privilege level
        uint32_t ts_ss0;
        uint32_t ts_esp1;
        uint32_t ts_ss1;
        uint32_t ts_esp2;
        uint32_t ts_ss2;
        // page directory base
        uint32_t ts_cr3;
        // saved state from last task switch
        uint32_t ts_eip;
        uint32_t ts_eflags;
        // more saved state (registers)
        uint32_t ts_eax;
        uint32_t ts_ecx;
        uint32_t ts_edx;
        uint32_t ts_ebx;
        uint32_t ts_esp;
        uint32_t ts_ebp;
        uint32_t ts_esi;
        uint32_t ts_edi;
        // even more saved state (segment selectors)
        uint32_t ts_es;
        uint32_t ts_cs;
        uint32_t ts_ss;
        uint32_t ts_ds;
        uint32_t ts_fs;
        uint32_t ts_gs;
        uint32_t ts_ldt;
        // trap on task switch
        uint32_t ts_t;
        // i/o map base address
        uint32_t ts_iomb;
    } __attribute__((packed)) tss_struct_t;
    // 全局描述符表构造函数，根据下标构造
    // 参数:
    // num-数组下标、base-基地址、limit-限长、access-访问标志，gran-粒度
    void set_gdt(int32_t num, uint32_t base, uint32_t limit, uint8_t access,
                 uint8_t gran);
    void set_tss(int32_t num, uint16_t ss0, uint32_t esp0);
    // 初始化
    int32_t init(void);
};

#endif /* _GDT_H_ */
