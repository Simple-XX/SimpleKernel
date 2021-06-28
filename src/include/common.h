
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// common.h for Simple-XX/SimpleKernel.

#ifndef _COMMON_H_
#define _COMMON_H_

#include "stdint.h"
#include "stddef.h"

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

    static constexpr const size_t BYTE = 0x1;
    static constexpr const size_t KB   = 0x400;
    static constexpr const size_t MB   = 0x100000;

    // 页大小 4KB
    static constexpr const size_t PAGE_SIZE = 4 * KB;

    // 对齐 向上取整
    // 针对指针
    template <class T>
    T ALIGN(const T _addr, const size_t _align) {
        uint8_t *tmp = (uint8_t *)_addr;
        return (T)((ptrdiff_t)(tmp + _align - 1) & (~(_align - 1)));
    }
    // 针对整数
    template <>
    uint64_t ALIGN(const uint64_t _x, const size_t _align) {
        return ((_x + _align - 1) & (~(_align - 1)));
    }

};

#endif /* _COMMON_H_ */
