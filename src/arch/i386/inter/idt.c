
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
// Based on http://wiki.0xffffff.org/posts/hurlex-6.html
// idt.c for MRNIU/SimpleKernel.

#include "idt.h"

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags){
  idt_entries[num].base_low = (base & 0xFFFF);
  idt_entries[num].base_high = (base >> 16) & 0xFFFF;
  idt_entries[num].selector = selector;
  idt_entries[num].zero = 0;
  idt_entries[num].flags = flags;
}

void idt_init(void){
  idt_ptr.limit = sizeof(idt_entry_t) * IDT_SIZE -1;
  idt_ptr.base = (uint32_t)&idt_entries;

  idt_set_gate(0x0, 0x0, 0x0, 0x0);  // Intel 文档要求首个描述符全 0
  for(int i=1; i<IDT_SIZE; i++){
      idt_set_gate(i, i, 0x08, 0x8E);
  }



  idt_load((uint32_t)&idt_ptr);
}
