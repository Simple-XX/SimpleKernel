
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
// boot .s for MRNIU/SimpleKernel.

#include "stdint.h"
#include "stdio.h"



uint8_t kern_stack[0x1000]  __attribute__ ( ( aligned(0x1000) ) );



uint32_t kern_stack_top = (uint32_t)kern_stack + 0x1000;



// 内核入口函数
__attribute__( ( section(".init.text") ) ) void kernel_entry(uint64_t magic, uint64_t addr)
{
    printk("-----------\n");
    printk("mag: %X, addr: %X\n", magic, addr);
    printk("-----------\n");
    kernel_main(magic, addr);
}
