
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

class KERNEL {
private:
    addr_t  magic;
    addr_t  addr;
    PMM     pmm;
    void    arch_init(void) const;
    int32_t test_pmm(void);

protected:
public:
    KERNEL(void);
    KERNEL(addr_t magic, addr_t addr);
    ~KERNEL(void);
    int32_t init(void);
    int32_t test(void);
    void    show_info(void);
};

#endif /* _KERNEL_HPP_ */
