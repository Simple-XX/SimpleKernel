
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// pmm.c for MRNIU/SimpleKernel.

#include "mm/pmm.h"


void show_memory_map(){
  uint32_t mmap_addr = glb_mboot_ptr->mmap_addr;
  uint32_t mmap_length = glb_mboot_ptr->mmap_length;

  mmap_entry_t *mmap = (mmap_entry_t *)mmap_addr;

  // 过滤掉最高位为 f 时的溢出问题(后面要改进)
  //mmap=((uint32_t)mmap)&0x0fffffff;
  mmap_addr&=0x0fffffff;
  mmap_length&=0x0fffffff;
  for(mmap;(((uint32_t)mmap)&0x0fffffff)<(mmap_addr+mmap_length);mmap++){
    printk("base_addr = 0x%X%08X, length = 0x%X%08X, type = 0x%X\n",
          (uint32_t)mmap->base_addr_high, (uint32_t)mmap->base_addr_low,
          (uint32_t)mmap->length_high, (uint32_t)mmap->length_low,
          (uint32_t)mmap->type);
  }
}
