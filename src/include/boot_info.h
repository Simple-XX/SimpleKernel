
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// boot_info.h for Simple-XX/SimpleKernel.

#ifndef _BOOT_INFO_H_
#define _BOOT_INFO_H_

#include "stdint.h"
#include "resource.h"

// 由引导传递的机器信息处理
// 如 grub2 传递的 multiboot2 结构
// opensbi 传递的 dtb 结构
// 注意这部分是通过内存传递的，在重新保存之前不能被覆盖
// 架构专有的数据在 dtb.h 或 multiboot2.h
// 实现在 dtb.cpp 或 multiboot2.cpp
namespace BOOT_INFO {
    // 声明，定义在具体的实现中
    // 地址
    extern "C" uintptr_t boot_info_addr;
    // 长度
    extern size_t boot_info_size;
    // 初始化
    extern bool init(void);
    // 获取物理内存信息
    extern resource_t get_memory(void);
    // 获取 clint
    extern resource_t get_clint(void);
    // 获取 plic
    extern resource_t get_plic(void);
};

#endif /* _BOOT_INFO_H_ */
