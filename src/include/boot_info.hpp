
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// boot_info.hpp for Simple-XX/SimpleKernel.

#ifndef _BOOT_INFO_HPP_
#define _BOOT_INFO_HPP_

#include "stdint.h"
#include "dtb.h"
#include "multiboot2.h"

// 由引导传递的机器信息处理
// 如 grub2 传递的 multiboot2 结构
// opensbi 传递的 dtb 结构
// 注意这部分是通过内存传递的，在重新保存之前不能被覆盖
namespace BOOT_INFO {
    // 声明，定义在具体的实现中
    // 地址
    extern "C" void *boot_info_addr;
    // 长度
    extern size_t boot_info_size;

// 根据不同的信息类型分别处理
#if defined(__i386__) || defined(__x86_64__)
    // 别名，统一接口
    using iter_data_t = MULTIBOOT2::multiboot2_iter_data_t;
    using iter_fun_t  = MULTIBOOT2::multiboot2_iter_fun_t;
    // 迭代器
    inline void iter(iter_fun_t _fun, void *_data) {
        MULTIBOOT2::multiboot2_iter(_fun, _data);
        return;
    }
    // 输出物理内存信息
    inline void printf_memory(void) {
        iter(MULTIBOOT2::printf_memory, nullptr);
        return;
    }
#elif defined(__riscv)
    // 别名，统一接口
    using iter_data_t = DTB::dtb_iter_data_t;
    using iter_fun_t  = DTB::dtb_iter_fun_t;
    // 迭代器
    inline void iter(iter_fun_t _fun, void *_data) {
        DTB::dtb_iter(_fun, _data);
        return;
    }
    // 输出物理内存信息
    inline void printf_memory(void) {
        iter(DTB::printf_memory, nullptr);
        return;
    }
#endif
};

#endif /* _BOOT_INFO_HPP_ */
