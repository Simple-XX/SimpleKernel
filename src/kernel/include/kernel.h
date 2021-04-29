
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.h for Simple-XX/SimpleKernel.

#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "stdint.h"

class KERNEL {
private:
    int32_t test_pmm(void);
    void    arch_init(void) const;

protected:
public:
    KERNEL(void);
    ~KERNEL(void);
    int32_t test(void);
    void    show_info(void);
};

#endif /* _KERNEL_H_ */
