
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.h for Simple-XX/SimpleKernel.

#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "stdint.h"
#include "pmm.h"
#include "io.h"

class KERNEL {
private:
    static IO io;
    PMM       pmm;
    uint32_t  magic;
    void *    addr;
    int32_t   test_pmm(void);

protected:
public:
    KERNEL(uint32_t _magic, void *_addr);
    ~KERNEL(void);
    int32_t test(void);
    void    show_info(void);
};

#endif /* _KERNEL_H_ */
