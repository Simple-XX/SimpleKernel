
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// pmm.c for MRNIU/SimpleKernel.


void show_memory_map(){
  uint32_t mmap_addr = glb_mboot_ptr->mmap_addr;
  uint32_t mmap_length = glb_mboot_ptr->mmap_length;

  mmap_entry_t *mmap = (mmap_entry_t *) mmap_addr;
  for(mmap=(mmap_entry_t *)mmap_addr;(uint32_t)mmap < mmap_addr+ mmap_length; mmap++){
    printk("%x\n",(uint32_t) mmap->base_addr_low);
  }
}
