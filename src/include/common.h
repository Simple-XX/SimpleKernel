
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// common.h for Simple-XX/SimpleKernel.

#ifndef _COMMON_H_
#define _COMMON_H_

#include "stdint.h"

namespace COMMON {
    // A common problem is getting garbage data when trying to use a value
    // defined in a linker script. This is usually because they're dereferencing
    // the symbol. A symbol defined in a linker script (e.g. _ebss = .;) is only
    // a symbol, not a variable. If you access the symbol using extern uint32_t
    // _ebss; and then try to use _ebss the code will try to read a 32-bit
    // integer from the address indicated by _ebss. The solution to this is to
    // take the address of _ebss either by using it as & _ebss or by defining it
    // as an unsized array(extern char _ebss[]; ) and casting to an integer.(The
    // array notation prevents accidental reads from _ebss as arrays must be
    // explicitly dereferenced) ref:
    // http://wiki.osdev.org/Using_Linker_Script_Values
    extern "C" void *kernel_start[];
    extern "C" void *kernel_text_start[];
    extern "C" void *kernel_text_end[];
    extern "C" void *kernel_data_start[];
    extern "C" void *kernel_data_end[];
    extern "C" void *kernel_end[];

    static const void *KERNEL_START_ADDR = kernel_start;
    static const void *KERNEL_TEXT_START_ADDR __attribute__((unused)) =
        kernel_text_start;
    static const void *KERNEL_TEXT_END_ADDR __attribute__((unused)) =
        kernel_text_end;
    static const void *KERNEL_DATA_START_ADDR __attribute__((unused)) =
        kernel_data_start;
    static const void *KERNEL_DATA_END_ADDR __attribute__((unused)) =
        kernel_data_end;
    static const void *KERNEL_END_ADDR = kernel_end;

// PAE 标志的处理
#ifdef CPU_PAE
#else
#endif

// PSE 标志的处理
#ifdef CPU_PSE
    // 页大小 4MB
    static constexpr const uint32_t PAGE_SIZE = 0x400000;
#else
    // 页大小 4KB
    static constexpr const uint32_t PAGE_SIZE = 0x1000;
#endif

    // 物理内存大小 2GB
    static constexpr const uint32_t PMM_MAX_SIZE = 0x80000000;

    // 物理页数量 131072,
    // 0x20000，除去外设映射，实际可用物理页数量为 159 + 130800 =
    // 130959 (这是物理内存为 512MB 的情况)
    static constexpr const uint32_t PMM_PAGE_MAX_SIZE =
        PMM_MAX_SIZE / PAGE_SIZE;

    // 页掩码，用于 4KB 对齐
    static constexpr const uint32_t PAGE_MASK = 0xFFFFF000;
    // 内核的偏移地址
    static constexpr const uint32_t KERNEL_BASE = 0x0;
    // 内核占用大小，与 KERNEL_START_ADDR，KERNEL_END_ADDR 无关
    // 64MB
    static constexpr const uint32_t KERNEL_SIZE = 0x4000000;
    // 映射内核需要的页数
    static constexpr const uint32_t KERNEL_PAGES = KERNEL_SIZE / PAGE_SIZE;

// 对齐 向上取整
#define ALIGN4K(x)                                                             \
    (((x) % PAGE_SIZE == 0) ? (x) : (((x) + PAGE_SIZE - 1) & PAGE_MASK))

    static const void *KERNEL_START_4K = reinterpret_cast<void *>(
        ALIGN4K(reinterpret_cast<uint32_t>(KERNEL_START_ADDR)));
    static const void *KERNEL_END_4K = reinterpret_cast<void *>(
        ALIGN4K(reinterpret_cast<uint32_t>(KERNEL_END_ADDR)));

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
};

#endif /* _COMMON_H_ */
