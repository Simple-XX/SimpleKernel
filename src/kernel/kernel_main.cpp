
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel_main.cpp for Simple-XX/SimpleKernel.

#include "cpu.hpp"
<<<<<<< HEAD
#include "debug.h"
=======
>>>>>>> intr
#include "assert.h"
#include "kernel.h"
#include "io.h"

<<<<<<< HEAD
extern "C" void kernel_main(addr_t magic, addr_t addr);

// 内核入口
void kernel_main(addr_t magic, addr_t addr) {
    KERNEL kernel(magic, addr);
    kernel.init();
    kernel.test();

=======
extern "C" void kernel_main(uint32_t magic, void *addr);

// 内核入口
void kernel_main(uint32_t magic, void *addr) {
    KERNEL kernel(magic, addr);
    kernel.show_info();
>>>>>>> intr
    CPU::sti();
    while (1) {
        ;
    }
    assert(0);
    return;
}
