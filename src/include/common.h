
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// common.h for Simple-XX/SimpleKernel.

#ifndef _COMMON_H_
#define _COMMON_H_

#include "stdint.h"
#include "stddef.h"

namespace COMMON {
    // 引用链接脚本中的变量
    // http://wiki.osdev.org/Using_Linker_Script_Values
    extern "C" void *kernel_start[];
    extern "C" void *kernel_text_start[];
    extern "C" void *kernel_text_end[];
    extern "C" void *kernel_data_start[];
    extern "C" void *kernel_data_end[];
    extern "C" void *kernel_end[];

    static const uintptr_t KERNEL_START_ADDR =
        reinterpret_cast<uintptr_t>(kernel_start);
    static const uintptr_t KERNEL_TEXT_START_ADDR __attribute__((unused)) =
        reinterpret_cast<uintptr_t>(kernel_text_start);
    static const uintptr_t KERNEL_TEXT_END_ADDR __attribute__((unused)) =
        reinterpret_cast<uintptr_t>(kernel_text_end);
    static const uintptr_t KERNEL_DATA_START_ADDR __attribute__((unused)) =
        reinterpret_cast<uintptr_t>(kernel_data_start);
    static const uintptr_t KERNEL_DATA_END_ADDR __attribute__((unused)) =
        reinterpret_cast<uintptr_t>(kernel_data_end);
    static const uintptr_t KERNEL_END_ADDR =
        reinterpret_cast<uintptr_t>(kernel_end);

    // 规定数据大小，方便用
    static constexpr const size_t BYTE = 0x1;
    static constexpr const size_t KB   = 0x400;
    static constexpr const size_t MB   = 0x100000;
    static constexpr const size_t GB   = 0x40000000;

    // 页大小 4KB
    static constexpr const size_t PAGE_SIZE = 4 * KB;
    // 内核空间占用大小，包括内核代码部分与预留的，8MB
    static constexpr const uint32_t KERNEL_SPACE_SIZE = 8 * MB;
    // 映射内核空间需要的页数
    static constexpr const uint64_t KERNEL_SPACE_PAGES =
        KERNEL_SPACE_SIZE / PAGE_SIZE;

    // 页掩码
    static constexpr const uintptr_t PAGE_MASK = ~(PAGE_SIZE - 1);

    // 对齐 向上取整
    // 针对指针
    template <class T>
    inline T ALIGN(const T _addr, size_t _align) {
        uint8_t *tmp = (uint8_t *)_addr;
        return (T)((uintptr_t)(tmp + _align - 1) & (~(_align - 1)));
    }
    // 针对整数
    template <>
    inline uint32_t ALIGN(uint32_t _x, size_t _align) {
        return ((_x + _align - 1) & (~(_align - 1)));
    }
    template <>
    inline uint64_t ALIGN(uint64_t _x, size_t _align) {
        return ((_x + _align - 1) & (~(_align - 1)));
    }

// 变量名转换为字符串
#define NAME2STR(_name) #_name
};

#endif /* _COMMON_H_ */
