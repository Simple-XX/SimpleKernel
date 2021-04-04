
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.h for Simple-XX/SimpleKernel.

#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "stdint.h"
#include "gdt.h"
#include "intr.h"
#include "apic.h"
#include "clock.h"
#include "pmm.h"
#include "io.h"

class KERNEL {
private:
    static IO   io;
    static APIC apic;
    PMM         pmm;
    uint32_t    magic;
    void *      addr;
    void        arch_init(void) const;
    void        drv_init(void) const;
    int32_t     test_pmm(void);

protected:
public:
    KERNEL(uint32_t _magic, void *_addr);
    ~KERNEL(void);
    int32_t test(void);
    void    show_info(void);
};

#endif /* _KERNEL_H_ */
