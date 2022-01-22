
/**
 * @file plic.cpp
 * @brief plic 抽象
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

#include "stdint.h"
#include "stdio.h"
#include "cpu.hpp"
#include "vmm.h"
#include "boot_info.h"
#include "io.h"
#include "intr.h"

/**
 * @brief 外部中断处理
 */
static void externel_intr(void) {
    // 读取中断号
    auto no = PLIC::get_instance().get();
    // 根据中断号判断设备
    printf("externel_intr: 0x%X.\n", no);
    return;
}

uint64_t PLIC::PLIC_SENABLE(uint64_t _hart) {
    return base_addr + 0x2080 + _hart * 0x100;
}

uint64_t PLIC::PLIC_SPRIORITY(uint64_t _hart) {
    return base_addr + 0x201000 + _hart * 0x2000;
}

uint64_t PLIC::PLIC_SCLAIM(uint64_t _hart) {
    return base_addr + 0x201004 + _hart * 0x2000;
}

PLIC &PLIC::get_instance(void) {
    /// 定义全局 PLIC 对象
    static PLIC plic;
    return plic;
}

int32_t PLIC::init(void) {
    // 初始化锁
    spinlock.init("PLIC");
    // 映射 plic
    resource_t resource = BOOT_INFO::get_plic();
    base_addr           = resource.mem.addr;
    PLIC_PRIORITY       = base_addr + 0x0;
    PLIC_PENDING        = base_addr + 0x1000;
    for (uintptr_t a = resource.mem.addr;
         a < resource.mem.addr + resource.mem.len; a += 0x1000) {
        VMM::get_instance().mmap(VMM::get_instance().get_pgd(), a, a,
                                 VMM_PAGE_READABLE | VMM_PAGE_WRITABLE);
    }
    // TODO: 多核情况下设置所有 hart
    // 将当前 hart 的 S 模式优先级阈值设置为 0
    IO::get_instance().write32(
        (void *)PLIC_SPRIORITY(CPU::get_curr_core_id()), 0);
    // 注册外部中断处理函数
    INTR::get_instance().register_interrupt_handler(INTR::INTR_S_EXTERNEL,
                                                    externel_intr);
    // 开启外部中断
    CPU::WRITE_SIE(CPU::READ_SIE() | CPU::SIE_SEIE);
    info("plic init.\n");
    return 0;
}

int32_t PLIC::init_other_core(void) {
    // 将当前 hart 的 S 模式优先级阈值设置为 0
    IO::get_instance().write32(
        (void *)PLIC_SPRIORITY(CPU::get_curr_core_id()), 0);
    // 开启外部中断
    CPU::WRITE_SIE(CPU::READ_SIE() | CPU::SIE_SEIE);
    info("plic other init.\n");
    return 0;
}

void PLIC::set(uint8_t _no, bool _status) {
    spinlock.lock();
    // 设置 IRQ 的属性为非零，即启用 plic
    IO::get_instance().write32((void *)(base_addr + _no * 4), _status);
    // TODO: 多核情况下设置所有 hart
    // 为当前 hart 的 S 模式设置 uart 的 enable
    if (_status) {
        IO::get_instance().write32(
            (void *)PLIC_SENABLE(BOOT_INFO::dtb_init_hart),
            IO::get_instance().read32(
                (void *)PLIC_SENABLE(BOOT_INFO::dtb_init_hart)) |
                (1 << _no));
    }
    else {
        IO::get_instance().write32(
            (void *)PLIC_SENABLE(BOOT_INFO::dtb_init_hart),
            IO::get_instance().read32(
                (void *)PLIC_SENABLE(BOOT_INFO::dtb_init_hart)) &
                ~(1 << _no));
    }
    spinlock.unlock();
    return;
}

uint8_t PLIC::get(void) {
    spinlock.lock();
    uint8_t ret = IO::get_instance().read32(
        (void *)PLIC_SCLAIM(BOOT_INFO::dtb_init_hart));
    spinlock.unlock();
    return ret;
}

void PLIC::done(uint8_t _no) {
    spinlock.lock();
    IO::get_instance().write32((void *)PLIC_SCLAIM(BOOT_INFO::dtb_init_hart),
                               _no);
    spinlock.unlock();
    return;
}
