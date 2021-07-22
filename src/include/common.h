
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

    static constexpr const void *KERNEL_START_ADDR = kernel_start;
    static constexpr const void *KERNEL_TEXT_START_ADDR
        __attribute__((unused)) = kernel_text_start;
    static constexpr const void *KERNEL_TEXT_END_ADDR __attribute__((unused)) =
        kernel_text_end;
    static constexpr const void *KERNEL_DATA_START_ADDR
        __attribute__((unused)) = kernel_data_start;
    static constexpr const void *KERNEL_DATA_END_ADDR __attribute__((unused)) =
        kernel_data_end;
    static constexpr const void *KERNEL_END_ADDR = kernel_end;

    // 规定数据大小，方便用
    static constexpr const size_t BYTE = 0x1;
    static constexpr const size_t KB   = 0x400;
    static constexpr const size_t MB   = 0x100000;
    static constexpr const size_t GB   = 0x40000000;

#if defined(__i386__)
    // 物理内存大小 2GB
    // TODO: 由引导程序传递
    static constexpr const uint32_t PMM_SIZE = 2 * GB;
    // 内核空间占用大小，包括内核代码部分与预留的
    // 64MB
    static constexpr const uint32_t KERNEL_SPACE_SIZE = 64 * MB;
    // 页大小 4KB
    static constexpr const size_t PAGE_SIZE = 4 * KB;
#elif defined(__x86_64__)
    // 物理内存大小 2GB
    // TODO: 由引导程序传递
    static constexpr const uint32_t PMM_SIZE = 2 * GB;
    // 内核空间占用大小，包括内核代码部分与预留的
    // 64MB
    static constexpr const uint32_t KERNEL_SPACE_SIZE = 64 * MB;
    // 页大小 2MB
    static constexpr const size_t PAGE_SIZE = 2 * MB;
#elif defined(__riscv)
    // 物理内存大小 128MB
    // TODO: 由引导程序传递
    static constexpr const uint64_t PMM_SIZE = 128 * MB;
    // 内核空间占用大小，包括内核代码部分与预留的
    // 8MB
    static constexpr const uint64_t KERNEL_SPACE_SIZE = 8 * MB;
    // 页大小 4KB
    static constexpr const size_t PAGE_SIZE = 4 * KB;
#endif

    // 物理页数量
    static constexpr const uint64_t PMM_PAGE_SIZE = PMM_SIZE / PAGE_SIZE;

    // 映射内核需要的页数
    static constexpr const uint64_t KERNEL_SPACE_PAGES =
        KERNEL_SPACE_SIZE / PAGE_SIZE;

    // 对齐 向上取整
    // 针对指针
    template <class T>
    inline T ALIGN(const T _addr, const size_t _align) {
        uint8_t *tmp = (uint8_t *)_addr;
        return (T)((uintptr_t)(tmp + _align - 1) & (~(_align - 1)));
    }
    // 针对整数
    template <>
    inline uint64_t ALIGN(const uint64_t _x, const size_t _align) {
        return ((_x + _align - 1) & (~(_align - 1)));
    }

};

#endif /* _COMMON_H_ */
