
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
// boot .s for MRNIU/SimpleKernel.

#include "stdint.h"

// #include "kernel.h"
//
// uint8_t kern_stack[0x1000]  __attribute__ ( ( aligned(0x1000) ) );
//
//
//
// uint32_t kern_stack_top = (uint32_t)kern_stack + 0x1000;
//


// 内核入口函数
__attribute__( ( section(".init.text") ) ) void kernel_entry(uint32_t magic, uint32_t addr)
{
    kernel_main(magic, addr);
}
