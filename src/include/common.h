
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// common.h for Simple-XX/SimpleKernel.

#ifndef _COMMON_H_
#define _COMMON_H_

#include "stdint.h"

namespace COMMON {

    extern "C" void *kernel_start[];
    extern "C" void *kernel_text_start[];
    extern "C" void *kernel_text_end[];
    extern "C" void *kernel_data_start[];
    extern "C" void *kernel_data_end[];
    extern "C" void *kernel_end[];

};

#endif /* _COMMON_H_ */
