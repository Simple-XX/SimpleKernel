
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// kernel.h for Simple-XX/SimpleKernel.

#ifndef _KERNEL_HPP_
#define _KERNEL_HPP_

#pragma once

#include "stdint.h"

extern "C" uint8_t kernel_start[];
extern "C" uint8_t kernel_text_start[];
extern "C" uint8_t kernel_text_end[];
extern "C" uint8_t kernel_data_start[];
extern "C" uint8_t kernel_data_end[];
extern "C" uint8_t kernel_end[];

class KERNEL {
private:
    void arch_init(void) const;
    void drv_init(void) const;

protected:
public:
    KERNEL(void);
    ~KERNEL(void);
    int32_t init(void);
    void    show_info(void);
};

#endif /* _KERNEL_HPP_ */
