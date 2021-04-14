
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// memlayout.h for Simple-XX/SimpleKernel.

#ifndef _MEMLAYOUT_H_
#define _MEMLAYOUT_H_

#include "stdint.h"

// 内存布局
namespace MEMLAYOUT {
    static constexpr const uint64_t DRAM_START = 0x80000000;
    static constexpr const uint64_t DRAM_END   = 0x88000000;
    static constexpr const uint64_t DRAM_SIZE  = DRAM_END - DRAM_START;
};

#endif /* _MEMLAYOUT_HPP_ */
