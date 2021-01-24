
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.h for Simple-XX/SimpleKernel.

#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "stdint.h"

extern "C" uint8_t kernel_start[];
extern "C" uint8_t kernel_text_start[];
extern "C" uint8_t kernel_text_end[];
extern "C" uint8_t kernel_data_start[];
extern "C" uint8_t kernel_data_end[];
extern "C" uint8_t kernel_end[];

extern "C" void kernel_main(void);

class KERNEL {
private:
#if defined(__arm__) || defined(__aarch__)

#endif
protected:
public:
    KERNEL(void);
    ~KERNEL(void);
    int  init(void);
    void show_info(void);
};

#endif /* _KERNEL_H_ */
