
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.h for Simple-XX/SimpleKernel.

#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "debug.h"
#include "gdt.h"
#include "intr.h"
#include "clock.h"

extern "C" void kernel_main(addr_t magic, addr_t addr);

class KERNEL {
private:
    void   arch_init(void) const;
    addr_t magic;
    addr_t addr;

protected:
public:
    KERNEL(void);
    KERNEL(addr_t magic, addr_t addr);
    ~KERNEL(void);
    void    show_info(void);
    int32_t init(void);
};

#endif /* _KERNEL_H_ */
