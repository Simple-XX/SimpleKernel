
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on http://wiki.0xffffff.org/posts/hurlex-kernel.html
// clint.cpp for Simple-XX/SimpleKernel.

#include "cpu.hpp"
#include "stdio.h"
#include "vmm.h"
#include "memlayout.h"
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

namespace CLINT {
    // 最大中断数
    static constexpr const uint32_t INTERRUPT_MAX = 16;
    // 最大异常数
    static constexpr const uint32_t EXCP_MAX = 16;
    // 中断处理函数数组
    static INTR::interrupt_handler_t interrupt_handlers[INTERRUPT_MAX]
        __attribute__((aligned(4)));
    // 异常处理函数数组
    static INTR::interrupt_handler_t excp_handlers[EXCP_MAX]
        __attribute__((aligned(4)));

    void register_interrupt_handler(uint8_t n, INTR::interrupt_handler_t h) {
        interrupt_handlers[n] = h;
        return;
    }

    void register_excp_handler(uint8_t n, INTR::interrupt_handler_t h) {
        excp_handlers[n] = h;
        return;
    }

    void do_interrupt(uint8_t n) {
        interrupt_handlers[n]();
        return;
    }

    void do_excp(uint8_t n) {
        excp_handlers[n]();
        return;
    }

    extern "C" void trap_entry(void);
    int32_t         init(void) {
        // 映射 clint 地址
        // TODO: 动态获取
        for (uint64_t a = MEMLAYOUT::CLINT; a < MEMLAYOUT::CLINT + 0x10000;
             a += 0x1000) {
            VMM::mmap(VMM::get_pgd(), (void *)a, (void *)a,
                      VMM_PAGE_READABLE | VMM_PAGE_WRITABLE);
        }
        // 设置 trap vector
        CPU::WRITE_STVEC((uint64_t)trap_entry);
        // 直接跳转到处理函数
        CPU::STVEC_DIRECT();
        // 设置处理函数
        for (auto &i : interrupt_handlers) {
            i = INTR::handler_default;
        }
        for (auto &i : excp_handlers) {
            i = INTR::handler_default;
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
};
