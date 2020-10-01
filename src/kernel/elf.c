
// This file is a part of SimpleXX/SimpleKernel
// (https://github.com/SimpleXX/SimpleKernel).
//
// elf.h for SimpleXX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "elf.h"
#include "stdbool.h"
#include "stddef.h"

// bool elf_check_file(struct multiboot_tag_elf_sections * tag) {
//   Elf32_Ehdr *hdr=(Elf32_Shdr *)tag;
//   // Elf32_Ehdr *hdr=(Elf32_Ehdr*)shdr->sh_addr;
//   // printk("e_ident: %c \n\t", ehdr->e_ident[EI_MAG1]);
//   if(!hdr) {
//     printk("Not ELF.\n");
//     return false;
//   }
//   if(hdr->e_ident[EI_MAG0] != ELFMAG0) {
//     printk("ELF Header EI_MAG0 incorrect.\n");
//     return false;
//   }
//   if(hdr->e_ident[EI_MAG1] != ELFMAG1) {
//     printk("ELF Header EI_MAG1 incorrect.\n");
//     return false;
//   }
//   if(hdr->e_ident[EI_MAG2] != ELFMAG2) {
//     printk("ELF Header EI_MAG2 incorrect.\n");
//     return false;
//   }
//   if(hdr->e_ident[EI_MAG3] != ELFMAG3) {
//     printk("ELF Header EI_MAG3 incorrect.\n");
//     return false;
//   }
//   return true;
// }
//
// bool elf_check_supported(Elf32_Ehdr *hdr) {
//   if(!elf_check_file(hdr)) {
//     printk("Invalid ELF File.\n");
//     return false;
//   }
//   if(hdr->e_ident[EI_CLASS] != ELFCLASS32) {
//     printk("Unsupported ELF File Class.\n");
//     return false;
//   }
//   if(hdr->e_ident[EI_DATA] != ELFDATA2LSB) {
//     printk("Unsupported ELF File byte order.\n");
//     return false;
//   }
//   if(hdr->e_machine != EM_386) {
//     printk("Unsupported ELF File target.\n");
//     return false;
//   }
//   if(hdr->e_ident[EI_VERSION] != EV_CURRENT) {
//     printk("Unsupported ELF File version.\n");
//     return false;
//   }
//   if(hdr->e_type != ET_REL && hdr->e_type != ET_EXEC) {
//     printk("Unsupported ELF File type.\n");
//     return false;
//   }
//   return true;
// }
//
// static inline Elf32_Shdr *elf_sheader(Elf32_Ehdr *hdr) {
//   return (Elf32_Shdr *)((int)hdr + hdr->e_shoff);
// }
//
// static inline Elf32_Shdr *elf_section(Elf32_Ehdr *hdr, int idx) {
//   return &elf_sheader(hdr)[idx];
// }
//
// static inline char *elf_str_table(Elf32_Ehdr *hdr) {
//   if(hdr->e_shstrndx == SHN_UNDEF) return NULL;
//   return (char *)hdr + elf_section(hdr, hdr->e_shstrndx)->sh_offset;
// }
//
// static inline char *elf_lookup_string(Elf32_Ehdr *hdr, int offset) {
//   char *strtab = elf_str_table(hdr);
//   if(strtab == NULL) return NULL;
//   return strtab + offset;
// }
//
// void elf_from_multiboot(uint32_t addr){
//   multiboot_tag_t *tag;
//   tag = (multiboot_tag_t *) (addr + 8);
//   tag = (multiboot_tag_t *) ((uint8_t *) tag + ((tag->size + 7) & ~7));
//
// }
//

#ifdef __cplusplus
}
#endif
