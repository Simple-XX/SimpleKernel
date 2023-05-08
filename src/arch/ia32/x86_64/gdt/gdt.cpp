
/**
 * @file gdt.cpp
 * @brief 描述符抽象实现
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

#include "stdio.h"
#include "gdt.h"

namespace GDT {
// 加载 GDTR
extern "C" void gdt_load(uint32_t);
// 全局 gdt 指针
static gdt_ptr64_t gdt_ptr64;
// 全局描述符表定义
static gdt_entry_t gdt_entries[GDT_LENGTH] __attribute__((aligned(8)));

void set_gdt(uint8_t _idx, uint32_t _base, uint32_t _limit, uint8_t _type,
             uint8_t _s, uint8_t _dpl, uint8_t _p, uint8_t _avl, uint8_t _l,
             uint8_t _db, uint8_t _g) {
    gdt_entries[_idx].limit1     = (_limit & 0xFFFF);
    gdt_entries[_idx].base_addr1 = (_base & 0xFFFF);
    gdt_entries[_idx].base_addr2 = (_base >> 16) & 0xFF;
    gdt_entries[_idx].type       = _type;
    gdt_entries[_idx].s          = _s;
    gdt_entries[_idx].dpl        = _dpl;
    gdt_entries[_idx].p          = _p;
    gdt_entries[_idx].limit2     = (_limit >> 16) & 0x0F;
    gdt_entries[_idx].avl        = _avl;
    gdt_entries[_idx].l          = _l;
    gdt_entries[_idx].db         = _db;
    gdt_entries[_idx].g          = _g;
    gdt_entries[_idx].base_addr3 = (_base >> 24) & 0xFF;
    return;
}

int32_t init(void) {
    // 全局描述符表界限  从 0 开始，所以总长要 - 1
    gdt_ptr64.limit = sizeof(gdt_entry_t) * GDT_LENGTH - 1;
    gdt_ptr64.base  = (uint64_t)&gdt_entries;
    // 采用 Intel 平坦模型
    // Intel 文档要求首个描述符全 0
    set_gdt(GDT_NULL, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0);
    // 内核指令段
    set_gdt(GDT_KERNEL_CODE, 0x0, 0x0, TYPE_CODE_EXECUTE_READ, S_CODE_DATA,
            CPU::DPL0, SEGMENT_PRESENT, 0x0, L_64BIT, 0x0, 0x0);
    // 内核数据段
    set_gdt(GDT_KERNEL_DATA, 0x0, 0x0, TYPE_DATA_READ_WRITE, S_CODE_DATA,
            CPU::DPL0, SEGMENT_PRESENT, 0x0, L_64BIT, 0x0, 0x0);
    // 用户模式代码段
    set_gdt(GDT_USER_CODE, 0x0, 0x0, TYPE_CODE_EXECUTE_READ, S_CODE_DATA,
            CPU::DPL3, SEGMENT_PRESENT, 0x0, L_64BIT, 0x0, 0x0);
    // 用户模式数据段
    set_gdt(GDT_USER_DATA, 0x0, 0x0, TYPE_DATA_READ_WRITE, S_CODE_DATA,
            CPU::DPL3, SEGMENT_PRESENT, 0x0, L_64BIT, 0x0, 0x0);
    // 加载全局描述符表地址到 GDTR 寄存器
    gdt_load((uint64_t)&gdt_ptr64);
    info("gdt init.\n");
    return 0;
}
}; // namespace GDT
