
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.h for Simple-XX/SimpleKernel.

#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "stdint.h"
#include "vfs.h"

class KERNEL {
private:
    VFS *   vfs;
    int32_t test_pmm(void);
    int32_t test_vmm(void);
    int32_t test_heap(void);
    int32_t test_vfs(void);

protected:
public:
    KERNEL(void);
    ~KERNEL(void);
    int32_t test(void);
    void    show_info(void);
};

#endif /* _KERNEL_H_ */
