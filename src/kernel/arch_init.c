
// This file is a part of MRNIU/SimpleKernel
// (https://github.com/MRNIU/SimpleKernel).
//
// arch_init.c for MRNIU/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "cpu.hpp"
#include "sync.hpp"
#include "intr/include/intr.h"
#include "gdt/include/gdt.h"
#include "arch_init.h"

void arch_init(void) {
    bool intr_flag = false;
    local_intr_store(intr_flag);
    {
        // GDT 初始化
        gdt_init();
        // IDT 初始化
        intr_init();
    }
    local_intr_restore(intr_flag);
    return;
}

#ifdef __cplusplus
}
#endif
