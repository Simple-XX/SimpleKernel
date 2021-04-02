
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.h for Simple-XX/SimpleKernel.

#ifndef _KERNEL_HPP_
#define _KERNEL_HPP_

#pragma once

#include "stdint.h"
#include "debug.h"
#include "gdt.h"
#include "intr.h"
#include "clock.h"
#include "pmm.h"
#include "vmm.h"

class KERNEL {
private:
    uint32_t magic;
    void *   addr;
    PMM      pmm;
    VMM      vmm;

    void    arch_init(void) const;
    int32_t test_pmm(void);
    int32_t test_vmm(void);

protected:
public:
    KERNEL(void);
    KERNEL(uint32_t magic, void *addr);
    ~KERNEL(void);
    int32_t init(void);
    int32_t test(void);
    void    show_info(void);
};

#endif /* _KERNEL_HPP_ */
