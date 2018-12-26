
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
// Based on https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#multiboot_002eh
// debug.c for MRNIU/SimpleKernel.

#include "debug.h"
#include "cpu.hpp"



/* Am I booted by a Multiboot-compliant boot loader? */
bool is_magic(uint32_t magic){
		if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
				printk ("Invalid magic number: 0x%x\n", (unsigned) magic);
				return false;
		}
		return true;
}

/* Print out the flags. */
void show_multiboot_flags(multiboot_info_t * mb){
		printk("flags = 0x%x\n", (unsigned) mb->flags);
		return;
}

/* Are mem_* valid? */
bool is_mem_vaild(multiboot_info_t * mb){
		if (CHECK_FLAG (mb->flags, 0)) {
				printk ("mem_lower = %uKB, mem_upper = %uKB\n",
				        (unsigned) mb->mem_lower, (unsigned) mb->mem_upper);
		}
}

/* Is boot_device valid? */
bool is_boot_device_vaild(multiboot_info_t * mb){
		if (CHECK_FLAG (mb->flags, 1)) {
				printk ("boot_device = 0x%x\n", (unsigned) mb->boot_device);
				return false;
		}
		return true;
}


/* Is the command line passed? */
bool is_command_line_passed(multiboot_info_t * mb){
		if (CHECK_FLAG (mb->flags, 2)) {
				printk ("cmdline = %s\n", (char *) mb->cmdline);
				return false;
		}
		return true;
}

/* Are mods_* valid? */
bool is_mods_valid(multiboot_info_t * mb){
		if (CHECK_FLAG (mb->flags, 3)) {
				multiboot_module_list_t *mod;
				int i;

				printk ("mods_count = %d, mods_addr = 0x%x\n",
				        (int) mb->mods_count, (int) mb->mods_addr);
				for (i = 0, mod = (multiboot_module_list_t *) mb->mods_addr;
				     i < mb->mods_count;
				     i++, mod++)
						printk (" mod_start = 0x%x, mod_end = 0x%x, cmdline = %s\n",
						        (unsigned) mod->mod_start,
						        (unsigned) mod->mod_end,
						        (char *) mod->cmdline);
				return false;
		}
		return true;
}

/* Bits 4 and 5 are mutually exclusive! */
bool is_bits4_bits5_both_set(multiboot_info_t * mb){
		if (CHECK_FLAG (mb->flags, 4) && CHECK_FLAG (mb->flags, 5)) {
				printk ("Both bits 4 and 5 are set.\n");
				return true;
		}
		return false;
}


/* Is the symbol table of a.out valid? */
bool is_symbol_table_aout_valid(multiboot_info_t * mb){
		if (CHECK_FLAG (mb->flags, 4)) {
				multiboot_aout_symbol_table_t *multiboot_aout_sym = &(mb->u.aout_sym);
				printk ("multiboot_aout_symbol_table:\n"
				        "tabsize = 0x%0x, strsize = 0x%x, addr = 0x%x\n",
				        (unsigned) multiboot_aout_sym->tabsize,
				        (unsigned) multiboot_aout_sym->strsize,
				        (unsigned) multiboot_aout_sym->addr);
				return true;
		}
		return false;
}

/* Is the section header table of ELF valid? */
bool is_section_header_table_elf_valid(multiboot_info_t * mb){
		if (CHECK_FLAG (mb->flags, 5)) {
				multiboot_elf_section_header_table_t *multiboot_elf_sec = &(mb->u.elf_sec);
				printk ("multiboot_elf_sec:\n"
				        "num = %u, size = 0x%x,\n addr = 0x%x, shndx = 0x%x\n",
				        (unsigned) multiboot_elf_sec->num, (unsigned) multiboot_elf_sec->size,
				        (unsigned) multiboot_elf_sec->addr, (unsigned) multiboot_elf_sec->shndx);
				return true;
		}
		return false;
}

/* Are mmap_* valid? */
bool is_mmap_valid(multiboot_info_t * mb){
		if (CHECK_FLAG (mb->flags, 6)) {
				multiboot_mmap_entry_t *mmap;

				printk ("mmap_addr = 0x%x, mmap_length = 0x%x\n",
				        (unsigned) mb->mmap_addr, (unsigned) mb->mmap_length);

// bug////////////////////////////////////////////////
				for (mmap = (multiboot_mmap_entry_t *) mb->mmap_addr;
				     (uint64_t) mmap < mb->mmap_addr + mb->mmap_length;
				     mmap = (multiboot_mmap_entry_t *) ((uint64_t) mmap + mmap->size + sizeof (mmap->size))
				     )
// bug////////////////////////////////////////////
						printk (" size = 0x%x, base_addr = 0x%x%08x,"
						        " length = 0x%x%08x, type = 0x%x\n",
						        (unsigned) mmap->size,
						        (unsigned) (mmap->addr >> 32),
						        (unsigned) (mmap->addr & 0xffffffff),
						        (unsigned) (mmap->len >> 32),
						        (unsigned) (mmap->len & 0xffffffff),
						        (unsigned) mmap->type);
				return true;
		}
		return false;
}


elf_info_t elf_from_multiboot(multiboot_info_t * mb){
		int i;
		elf_info_t elf;
		elf32_section_header_t * sh = (elf32_section_header_t *)mb->u.elf_sec.addr;
		uint32_t shstrtab = sh[mb->u.elf_sec.shndx].addr;
// bug //////////////////////////////////////////////
		for(i=0; i < mb->u.elf_sec.num; i++) {
				const char *name = (const char *)(shstrtab + sh[i].name);
				// printk_color(red, "w");
				printk_color(red, "num: %d ", mb->u.elf_sec.num);
				printk_color(red, "size: %d ", mb->u.elf_sec.size);
				printk_color(red, "i: %d ",i);
				// 在 GRUB 提供的 multiboot 信息中寻找内核 ELF 格式所提取的字符串表和符号表
				if (strcmp(name, ".strtab") == 0) {
						printk_color(red, "33333333333");
						elf.strtab = (const char *)sh[i].addr;
						elf.strtabsz = sh[i].size;
				}
				if (strcmp(name, ".symtab") == 0) {
						printk_color(red, "444444444444");
						elf.symtab = (elf_symbol_t * )sh[i].addr;
						elf.symtabsz = sh[i].size;
				}
		}
// bug //////////////////////////////////////////////
		return elf;
}

const char *elf_lookup_symbol(uint32_t addr, elf_info_t *elf){
		int i;
		for (i = 0; i < (elf->symtabsz / sizeof(elf_symbol_t)); i++) {
				if (ELF32_ST_TYPE(elf->symtab[i].info) != 0x2) {
						continue;
				}
				// 通过函数调用地址查到函数的名字
				if ( (addr >= elf->symtab[i].value) && (addr < (elf->symtab[i].value + elf->symtab[i].size)) ) {
						return (const char *)((uint32_t)elf->strtab + elf->symtab[i].name);
				}
		}
		return NULL;
}

void print_cur_status(){
		static int round = 0;
		uint16_t reg1, reg2, reg3, reg4;
		asm volatile (  "mov %%cs, %0;"
		                "mov %%ds, %1;"
		                "mov %%es, %2;"
		                "mov %%ss, %3;"
		                : "=m" (reg1), "=m" (reg2), "=m" (reg3), "=m" (reg4));

		// 打印当前的运行级别
		printk("%d:  @ring %d\n", round, reg1 & 0x3);
		printk("%d:  cs = %x\n", round, reg1);
		printk("%d:  ds = %x\n", round, reg2);
		printk("%d:  es = %x\n", round, reg3);
		printk("%d:  ss = %x\n", round, reg4);
		++round;
}

static elf_info_t kernel_elf;
void debug_init(uint64_t magic, multiboot_info_t * mb) {


		// 从 GRUB 提供的信息中获取到内核符号表和代码地址信息
		// kernel_elf = elf_from_multiboot(mb);

		is_magic(magic);
		show_multiboot_flags(mb);
		is_mem_vaild(mb);
		is_boot_device_vaild(mb);
		is_command_line_passed(mb);
		is_mods_valid(mb);
		is_bits4_bits5_both_set(mb);
		is_symbol_table_aout_valid(mb);
		is_section_header_table_elf_valid(mb);
		is_mmap_valid(mb);
		print_cur_status();
		return;
}

void print_stack_trace() {
		uint32_t *ebp, *eip;

		asm volatile ("mov %%ebp, %0" : "=r" (ebp));
		while (ebp) {
				eip = ebp + 1;
				printk("   [0x%x] %s\n", *eip, elf_lookup_symbol(*eip, &kernel_elf));
				ebp = (uint32_t*)*ebp;
		}
}

void panic(const char *msg) {
		printk("*** System panic: %s\n", msg);
		print_stack_trace();
		printk("***\n");

		// 致命错误发生后打印栈信息后停止在这里
		while(1)
				cpu_hlt();
}
