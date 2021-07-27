
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// boot_info.hpp for Simple-XX/SimpleKernel.

#ifndef _BOOT_INFO_HPP_
#define _BOOT_INFO_HPP_

#include "stdint.h"
#include "dtb.h"
#include "multiboot2.h"

namespace BOOT_INFO {
    // 声明，定义在具体的实现中
    // 地址
    extern "C" void *boot_info_addr;
    // 长度
    extern size_t boot_info_size;

#if defined(__i386__) || defined(__x86_64__)
    using iter_data_t = MULTIBOOT2::multiboot2_iter_data_t;
    using iter_fun_t  = MULTIBOOT2::multiboot2_iter_fun_t;
    void iter(iter_fun_t _fun, void *_data) {
        MULTIBOOT2::multiboot2_iter(_fun, _data);
        return;
    }
#elif defined(__riscv)
    using iter_data_t = DTB::dtb_iter_data_t;
    using iter_fun_t  = DTB::dtb_iter_fun_t;
    inline void iter(iter_fun_t _fun, void *_data) {
        DTB::dtb_iter(_fun, _data);
        return;
    }
#endif
};

#endif /* _BOOT_INFO_HPP_ */
