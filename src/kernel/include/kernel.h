
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.h for Simple-XX/SimpleKernel.

#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "stdint.h"
#include "debug.h"
#include "gdt.h"
#include "intr.h"
#include "clock.h"

extern "C" void *kernel_start[];
extern "C" void *kernel_text_start[];
extern "C" void *kernel_text_end[];
extern "C" void *kernel_data_start[];
extern "C" void *kernel_data_end[];
extern "C" void *kernel_end[];

extern "C" void kernel_main(void);

class KERNEL {
private:
    void arch_init(void) const;

protected:
public:
    KERNEL(void);
    ~KERNEL(void);
    int32_t init(void);
    void    show_info(void);
};

#endif /* _KERNEL_H_ */
