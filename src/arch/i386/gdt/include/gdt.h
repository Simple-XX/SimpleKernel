
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on http://wiki.0xffffff.org/posts/hurlex-6.html
// gdt.h for Simple-XX/SimpleKernel.

#ifndef _GDT_H_
#define _GDT_H_

#pragma once

#include "stdint.h"

namespace GDT {
    // 全局描述符表长度
    static constexpr const uint32_t GDT_LENGTH = 6;
    // 各个内存段所在全局描述符表下标
    static constexpr const uint32_t SEG_NULL  = 0;
    static constexpr const uint32_t SEG_KTEXT = 1;
    static constexpr const uint32_t SEG_KDATA = 2;
    static constexpr const uint32_t SEG_UTEXT = 3;
    static constexpr const uint32_t SEG_UDATA = 4;
    static constexpr const uint32_t SEG_TSS   = 5;
    // 内核代码段 0x08
    static constexpr const uint32_t GD_KTEXT = SEG_KTEXT << 3;
    // 内核数据段
    static constexpr const uint32_t GD_KDATA = SEG_KDATA << 3;
    // 用户代码段
    static constexpr const uint32_t GD_UTEXT = SEG_UTEXT << 3;
    // 用户数据段
    static constexpr const uint32_t GD_UDATA = SEG_UDATA << 3;
    // 任务段
    static constexpr const uint32_t GD_TSS = SEG_TSS << 3;
    // 段描述符 DPL
    // 内核级
    static constexpr const uint32_t DPL_KERNEL = 0;
    // 用户级
    static constexpr const uint32_t DPL_USER = 3;

    // 各个段的全局描述符表的选择子
    static constexpr const uint32_t KERNEL_CS = GD_KTEXT | DPL_KERNEL;
    static constexpr const uint32_t KERNEL_DS = GD_KDATA | DPL_KERNEL;
    static constexpr const uint32_t USER_CS   = GD_UTEXT | DPL_USER;
    static constexpr const uint32_t USER_DS   = GD_UDATA | DPL_USER;

    // 访问权限
    static constexpr const uint32_t KREAD_EXEC  = 0x9A;
    static constexpr const uint32_t KREAD_WRITE = 0x92;
    static constexpr const uint32_t UREAD_EXEC  = 0xFA;
    static constexpr const uint32_t UREAD_WRITE = 0xF2;

    // 全局描述符类型
    typedef struct gdt_entry {
        // 段界限   15～0
        uint16_t limit_low;
        // 段基地址 15～0
        uint16_t base_low;
        // 段基地址 23～16
        uint8_t base_middle;
        // 段存在位、描述符特权级、描述符类型、描述符子类别
        uint8_t access;
        // 其他标志、段界限 19～16 (unsigned limit_high:
        // 4;unsigned flags: 4;)
        uint8_t granularity;
        // 段基地址 31～24
        uint8_t base_high;
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
