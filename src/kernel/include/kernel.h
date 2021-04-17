
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.h for Simple-XX/SimpleKernel.

#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "stdint.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"
#include "io.h"

class KERNEL {
private:
    static IO io;
    PMM       pmm;
    VMM       vmm;
    HEAP      heap;
    int32_t   test_pmm(void);
    int32_t   test_vmm(void);
    int32_t   test_heap(void);

protected:
public:
    KERNEL(void);
    ~KERNEL(void);
    int32_t test(void);
    void    show_info(void);
};

#endif /* _KERNEL_H_ */
