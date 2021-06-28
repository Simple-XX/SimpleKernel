
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

#if defined(__i386__) || defined(__x86_64__)
    // 物理内存大小 2GB
    static constexpr const uint32_t PMM_MAX_SIZE = 0x80000000;
    // 内核的偏移地址
    static constexpr const uint32_t KERNEL_BASE = 0x0;
    // 内核占用大小，与 KERNEL_START_ADDR，KERNEL_END_ADDR 无关
    // 64MB
    static constexpr const uint32_t KERNEL_SIZE = 0x4000000;

    // 页大小 4KB
    static constexpr const size_t PAGE_SIZE = 4 * KB;
#elif defined(__riscv)
    // 物理内存大小 128MB
    static constexpr const uint64_t PMM_MAX_SIZE = 0x8000000;
    // 内核的偏移地址
    static constexpr const uint64_t KERNEL_BASE = 0x80020000;
    // 内核占用大小，与 KERNEL_START_ADDR，KERNEL_END_ADDR 无关
    // 8MB
    static constexpr const uint64_t KERNEL_SIZE = 0x800000;
    // 页大小 4KB
    static constexpr const size_t PAGE_SIZE = 4 * KB;
#endif

    // 页掩码，用于 4KB 对齐
    static constexpr const uint64_t PAGE_MASK =
        0xFFFFFFFFFFFFFFFF - PAGE_SIZE + 1;
    // 物理页数量 131072,
    // 0x20000，除去外设映射，实际可用物理页数量为 159 + 130800 =
    // 130959 (这是物理内存为 512MB 的情况)
    static constexpr const uint64_t PMM_PAGE_MAX_SIZE =
        PMM_MAX_SIZE / PAGE_SIZE;

    // 映射内核需要的页数
    static constexpr const uint64_t KERNEL_PAGES = KERNEL_SIZE / PAGE_SIZE;

    // 物理页结构体
    class physical_pages_t {
    public:
        // 起始地址
        uint8_t *addr;
        // 该页被引用次数，-1 表示此页内存被保留，禁止使用
        int32_t ref;
    };

    // zone 机制
    // NOTE: 这里的 ZONE 与 Linux 中的划分不同，只是方便控制分配内存的位置
    enum zone_t : uint8_t {
        // 与 KERNEL_SIZE 对应
        NORMAL = 0,
        // 高于 KERNEL_SIZE 的地址
        HIGH,
    };

    static constexpr const size_t ZONE_COUNT = 2;

    // 对齐 向上取整
    // 针对指针
    template <class T>
    inline T ALIGN(const T _addr, const size_t _align) {
        uint8_t *tmp = (uint8_t *)_addr;
        return (T)((ptrdiff_t)(tmp + _align - 1) & (~(_align - 1)));
    }
    // 针对整数
    template <>
    inline uint64_t ALIGN(const uint64_t _x, const size_t _align) {
        return ((_x + _align - 1) & (~(_align - 1)));
    }

};

#endif /* _COMMON_H_ */
