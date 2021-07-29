
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on http://wiki.0xffffff.org/posts/hurlex-kernel.html
// clint.cpp for Simple-XX/SimpleKernel.

#include "cpu.hpp"
#include "stdio.h"
#include "pmm.h"
#include "vmm.h"
#include "boot_info.hpp"
#include "intr.h"
#include "cpu.hpp"

// 缺页处理
static void pg_load_excp(void) {
    void *addr = (void *)CPU::READ_STVAL();
    // 映射页
    VMM::mmap(VMM::get_pgd(), addr, addr, VMM_PAGE_READABLE);
    info("pg_load_excp done: 0x%p.\n", addr);
    return;
}

static void pg_store_excp(void) {
    void *addr = (void *)CPU::READ_STVAL();
    // 映射页
    VMM::mmap(VMM::get_pgd(), addr, addr,
              VMM_PAGE_WRITABLE | VMM_PAGE_READABLE);
    info("pg_store_excp done: 0x%p.\n", addr);
    return;
}

extern "C" void trap_entry(void);

// 中断处理函数数组
INTR::interrupt_handler_t CLINT::interrupt_handlers[CLINT::INTERRUPT_MAX];
// 异常处理函数数组
INTR::interrupt_handler_t CLINT::excp_handlers[CLINT::EXCP_MAX];

void CLINT::register_interrupt_handler(
    uint8_t _no, INTR::interrupt_handler_t _interrupt_handler) {
    interrupt_handlers[_no] = _interrupt_handler;
    return;
}

void CLINT::register_excp_handler(
    uint8_t _no, INTR::interrupt_handler_t _interrupt_handler) {
    excp_handlers[_no] = _interrupt_handler;
    return;
}

void CLINT::do_interrupt(uint8_t _no) {
    interrupt_handlers[_no]();
    return;
}

void CLINT::do_excp(uint8_t _no) {
    excp_handlers[_no]();
    return;
}

static PMM::phy_mem_t clint_phy_mem;

int32_t CLINT::init(void) {
    // 映射 clint 地址
    BOOT_INFO::get_clint(&clint_phy_mem);
    for (uintptr_t a = (uintptr_t)clint_phy_mem.addr;
         a < ((uintptr_t)clint_phy_mem.addr) + clint_phy_mem.len; a += 0x1000) {
        VMM::mmap(VMM::get_pgd(), (void *)a, (void *)a,
                  VMM_PAGE_READABLE | VMM_PAGE_WRITABLE);
    }
    // 设置 trap vector
    CPU::WRITE_STVEC((uintptr_t)trap_entry);
    // 直接跳转到处理函数
    CPU::STVEC_DIRECT();
    // 设置处理函数
    for (auto &i : interrupt_handlers) {
        i = handler_default;
    }
    for (auto &i : excp_handlers) {
        i = handler_default;
    }
    // 缺页中断
    register_excp_handler(EXCP_LOAD_PAGE_FAULT, pg_load_excp);
    register_excp_handler(EXCP_STORE_PAGE_FAULT, pg_store_excp);
    // 开启内部中断
    CPU::WRITE_SIE(CPU::READ_SIE() | CPU::SIE_SSIE);
    // 设置时钟中断
    TIMER::init();
    printf("clint init.\n");
    return 0;
}
