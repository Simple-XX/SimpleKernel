
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// multiboot2.h for Simple-XX/SimpleKernel.

#ifndef _MULTIBOOT2_H_
#define _MULTIBOOT2_H_

#include "stdint.h"
#include "stdbool.h"
#include "e820.h"

// 启动后，在 32 位内核进入点，机器状态如下：
//   1. CS 指向基地址为 0x00000000，限长为4G – 1的代码段描述符。
//   2. DS，SS，ES，FS 和 GS 指向基地址为0x00000000，限长为4G –
//   1的数据段描述符。
//   3. A20 地址线已经打开。
//   4. 页机制被禁止。
//   5. 中断被禁止。
//   6. EAX = 0x2BADB002
//   7. 系统信息和启动信息块的线性地址保存在 EBX中（相当于一个指针）。
//      以下即为这个信息块的结构

namespace MULTIBOOT2 {
    /*  How many bytes from the start of the file we search for the header. */
    static constexpr const uint32_t MULTIBOOT_SEARCH       = 32768;
    static constexpr const uint32_t MULTIBOOT_HEADER_ALIGN = 8;

    /*  The magic field should contain this. */
    static constexpr const uint32_t MULTIBOOT2_HEADER_MAGIC = 0xe85250d6;

    /*  This should be in %eax. */
    static constexpr const uint32_t MULTIBOOT2_BOOTLOADER_MAGIC = 0x36d76289;

    /*  Alignment of multiboot modules. */
    static constexpr const uint32_t MULTIBOOT_MOD_ALIGN = 0x00001000;

    /*  Alignment of the multiboot info structure. */
    static constexpr const uint32_t MULTIBOOT_INFO_ALIGN = 0x00000008;

    /*  Flags set in the 'flags' member of the multiboot header. */

    static constexpr const uint32_t MULTIBOOT_TAG_ALIGN                 = 8;
    static constexpr const uint32_t MULTIBOOT_TAG_TYPE_END              = 0;
    static constexpr const uint32_t MULTIBOOT_TAG_TYPE_CMDLINE          = 1;
    static constexpr const uint32_t MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME = 2;
    static constexpr const uint32_t MULTIBOOT_TAG_TYPE_MODULE           = 3;
    static constexpr const uint32_t MULTIBOOT_TAG_TYPE_BASIC_MEMINFO    = 4;
    static constexpr const uint32_t MULTIBOOT_TAG_TYPE_BOOTDEV          = 5;
    static constexpr const uint32_t MULTIBOOT_TAG_TYPE_MMAP             = 6;
    static constexpr const uint32_t MULTIBOOT_TAG_TYPE_VBE              = 7;
    static constexpr const uint32_t MULTIBOOT_TAG_TYPE_FRAMEBUFFER      = 8;
    static constexpr const uint32_t MULTIBOOT_TAG_TYPE_ELF_SECTIONS     = 9;
    static constexpr const uint32_t MULTIBOOT_TAG_TYPE_APM              = 10;
    static constexpr const uint32_t MULTIBOOT_TAG_TYPE_EFI32            = 11;
    static constexpr const uint32_t MULTIBOOT_TAG_TYPE_EFI64            = 12;
    static constexpr const uint32_t MULTIBOOT_TAG_TYPE_SMBIOS           = 13;
    static constexpr const uint32_t MULTIBOOT_TAG_TYPE_ACPI_OLD         = 14;
    static constexpr const uint32_t MULTIBOOT_TAG_TYPE_ACPI_NEW         = 15;
    static constexpr const uint32_t MULTIBOOT_TAG_TYPE_NETWORK          = 16;
    static constexpr const uint32_t MULTIBOOT_TAG_TYPE_EFI_MMAP         = 17;
    static constexpr const uint32_t MULTIBOOT_TAG_TYPE_EFI_BS           = 18;
    static constexpr const uint32_t MULTIBOOT_TAG_TYPE_EFI32_IH         = 19;
    static constexpr const uint32_t MULTIBOOT_TAG_TYPE_EFI64_IH         = 20;
    static constexpr const uint32_t MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR   = 21;

    static constexpr const uint32_t MULTIBOOT_HEADER_TAG_END = 0;
    static constexpr const uint32_t MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST =
        1;
    static constexpr const uint32_t MULTIBOOT_HEADER_TAG_ADDRESS       = 2;
    static constexpr const uint32_t MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS = 3;
    static constexpr const uint32_t MULTIBOOT_HEADER_TAG_CONSOLE_FLAGS = 4;
    static constexpr const uint32_t MULTIBOOT_HEADER_TAG_FRAMEBUFFER   = 5;
    static constexpr const uint32_t MULTIBOOT_HEADER_TAG_MODULE_ALIGN  = 6;
    static constexpr const uint32_t MULTIBOOT_HEADER_TAG_EFI_BS        = 7;
    static constexpr const uint32_t MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI32 =
        8;
    static constexpr const uint32_t MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI64 =
        9;
    static constexpr const uint32_t MULTIBOOT_HEADER_TAG_RELOCATABLE = 10;

    static constexpr const uint32_t MULTIBOOT_ARCHITECTURE_I386   = 0;
    static constexpr const uint32_t MULTIBOOT_ARCHITECTURE_MIPS32 = 4;
    static constexpr const uint32_t MULTIBOOT_HEADER_TAG_OPTIONAL = 1;

    static constexpr const uint32_t MULTIBOOT_LOAD_PREFERENCE_NONE = 0;
    static constexpr const uint32_t MULTIBOOT_LOAD_PREFERENCE_LOW  = 1;
    static constexpr const uint32_t MULTIBOOT_LOAD_PREFERENCE_HIGH = 2;

    static constexpr const uint32_t MULTIBOOT_CONSOLE_FLAGS_CONSOLE_REQUIRED =
        1;
    static constexpr const uint32_t MULTIBOOT_CONSOLE_FLAGS_EGA_TEXT_SUPPORTED =
        2;

    typedef uint8_t  multiboot_uint8_t;
    typedef uint16_t multiboot_uint16_t;
    typedef uint32_t multiboot_uint32_t;
    typedef uint64_t multiboot_uint64_t;

    struct multiboot_header {
        // Must be MULTIBOOT_MAGIC - see above.
        multiboot_uint32_t magic;
        // ISA
        multiboot_uint32_t architecture;
        // Total header length.
        multiboot_uint32_t header_length;
        // The above fields plus this one must equal 0 mod 2^32.
        multiboot_uint32_t checksum;
    };

    struct multiboot_header_tag {
        multiboot_uint16_t type;
        multiboot_uint16_t flags;
        multiboot_uint32_t size;
    };

    struct multiboot_header_tag_information_request {
        multiboot_uint16_t type;
        multiboot_uint16_t flags;
        multiboot_uint32_t size;
        multiboot_uint32_t requests[0];
    };

    struct multiboot_header_tag_address {
        multiboot_uint16_t type;
        multiboot_uint16_t flags;
        multiboot_uint32_t size;
        multiboot_uint32_t header_addr;
        multiboot_uint32_t load_addr;
        multiboot_uint32_t load_end_addr;
        multiboot_uint32_t bss_end_addr;
    };

    struct multiboot_header_tag_entry_address {
        multiboot_uint16_t type;
        multiboot_uint16_t flags;
        multiboot_uint32_t size;
        multiboot_uint32_t entry_addr;
    };

    struct multiboot_header_tag_console_flags {
        multiboot_uint16_t type;
        multiboot_uint16_t flags;
        multiboot_uint32_t size;
        multiboot_uint32_t console_flags;
    };

    struct multiboot_header_tag_framebuffer {
        multiboot_uint16_t type;
        multiboot_uint16_t flags;
        multiboot_uint32_t size;
        multiboot_uint32_t width;
        multiboot_uint32_t height;
        multiboot_uint32_t depth;
    };

    struct multiboot_header_tag_module_align {
        multiboot_uint16_t type;
        multiboot_uint16_t flags;
        multiboot_uint32_t size;
    };

    struct multiboot_header_tag_relocatable {
        multiboot_uint16_t type;
        multiboot_uint16_t flags;
        multiboot_uint32_t size;
        multiboot_uint32_t min_addr;
        multiboot_uint32_t max_addr;
        multiboot_uint32_t align;
        multiboot_uint32_t preference;
    };

    struct multiboot_color {
        multiboot_uint8_t red;
        multiboot_uint8_t green;
        multiboot_uint8_t blue;
    };

    static constexpr const uint32_t MULTIBOOT_MEMORY_AVAILABLE        = 1;
    static constexpr const uint32_t MULTIBOOT_MEMORY_RESERVED         = 2;
    static constexpr const uint32_t MULTIBOOT_MEMORY_ACPI_RECLAIMABLE = 3;
    static constexpr const uint32_t MULTIBOOT_MEMORY_NVS              = 4;
    static constexpr const uint32_t MULTIBOOT_MEMORY_BADRAM           = 5;
    struct multiboot_mmap_entry {
        multiboot_uint64_t addr;
        multiboot_uint64_t len;
        multiboot_uint32_t type;
        multiboot_uint32_t zero;
    };
    typedef struct multiboot_mmap_entry multiboot_memory_map_entry_t;

    struct multiboot_tag {
        multiboot_uint32_t type;
        multiboot_uint32_t size;
    };
    typedef struct multiboot_tag multiboot_tag_t;
    typedef struct multiboot_tag multiboot_mmap_tag_t;

    struct multiboot_tag_string {
        multiboot_uint32_t type;
        multiboot_uint32_t size;
        char               string[0];
    };

    struct multiboot_tag_module {
        multiboot_uint32_t type;
        multiboot_uint32_t size;
        multiboot_uint32_t mod_start;
        multiboot_uint32_t mod_end;
        char               cmdline[0];
    };

    struct multiboot_tag_basic_meminfo {
        multiboot_uint32_t type;
        multiboot_uint32_t size;
        multiboot_uint32_t mem_lower;
        multiboot_uint32_t mem_upper;
    };

    struct multiboot_tag_bootdev {
        multiboot_uint32_t type;
        multiboot_uint32_t size;
        multiboot_uint32_t biosdev;
        multiboot_uint32_t slice;
        multiboot_uint32_t part;
    };

    typedef struct {
        multiboot_uint32_t          type;
        multiboot_uint32_t          size;
        multiboot_uint32_t          entry_size;
        multiboot_uint32_t          entry_version;
        struct multiboot_mmap_entry entries[0];
    } multiboot_tag_mmap_t;

    struct multiboot_vbe_info_block {
        multiboot_uint8_t external_specification[512];
    };

    struct multiboot_vbe_mode_info_block {
        multiboot_uint8_t external_specification[256];
    };

    struct multiboot_tag_vbe {
        multiboot_uint32_t type;
        multiboot_uint32_t size;

        multiboot_uint16_t vbe_mode;
        multiboot_uint16_t vbe_interface_seg;
        multiboot_uint16_t vbe_interface_off;
        multiboot_uint16_t vbe_interface_len;

        struct multiboot_vbe_info_block      vbe_control_info;
        struct multiboot_vbe_mode_info_block vbe_mode_info;
    };

    struct multiboot_tag_elf_sections {
        // 0x09
        multiboot_uint32_t type;
        multiboot_uint32_t size;
        multiboot_uint32_t num;
        multiboot_uint32_t entsize;
        // 段字符串表索引
        multiboot_uint32_t shndx;
        char               sections[0];
    };

    struct multiboot_tag_apm {
        multiboot_uint32_t type;
        multiboot_uint32_t size;
        multiboot_uint16_t version;
        multiboot_uint16_t cseg;
        multiboot_uint32_t offset;
        multiboot_uint16_t cseg_16;
        multiboot_uint16_t dseg;
        multiboot_uint16_t flags;
        multiboot_uint16_t cseg_len;
        multiboot_uint16_t cseg_16_len;
        multiboot_uint16_t dseg_len;
    };

    struct multiboot_tag_efi32 {
        multiboot_uint32_t type;
        multiboot_uint32_t size;
        multiboot_uint32_t pointer;
    };

    struct multiboot_tag_efi64 {
        multiboot_uint32_t type;
        multiboot_uint32_t size;
        multiboot_uint64_t pointer;
    };

    struct multiboot_tag_smbios {
        multiboot_uint32_t type;
        multiboot_uint32_t size;
        multiboot_uint8_t  major;
        multiboot_uint8_t  minor;
        multiboot_uint8_t  reserved[6];
        multiboot_uint8_t  tables[0];
    };

    struct multiboot_tag_old_acpi {
        multiboot_uint32_t type;
        multiboot_uint32_t size;
        multiboot_uint8_t  rsdp[0];
    };

    struct multiboot_tag_new_acpi {
        multiboot_uint32_t type;
        multiboot_uint32_t size;
        multiboot_uint8_t  rsdp[0];
    };

    struct multiboot_tag_network {
        multiboot_uint32_t type;
        multiboot_uint32_t size;
        multiboot_uint8_t  dhcpack[0];
    };

    struct multiboot_tag_efi_mmap {
        multiboot_uint32_t type;
        multiboot_uint32_t size;
        multiboot_uint32_t descr_size;
        multiboot_uint32_t descr_vers;
        multiboot_uint8_t  efi_mmap[0];
    };

    struct multiboot_tag_efi32_ih {
        multiboot_uint32_t type;
        multiboot_uint32_t size;
        multiboot_uint32_t pointer;
    };

    struct multiboot_tag_efi64_ih {
        multiboot_uint32_t type;
        multiboot_uint32_t size;
        multiboot_uint64_t pointer;
    };

    struct multiboot_tag_load_base_addr {
        multiboot_uint32_t type;
        multiboot_uint32_t size;
        multiboot_uint32_t load_base_addr;
    };

    extern "C" void multiboot2_init(uint32_t magic, void *addr);
    void            print_MULTIBOOT_TAG_TYPE_CMDLINE(struct multiboot_tag *tag);
    void print_MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME(struct multiboot_tag *tag);
    void print_MULTIBOOT_TAG_TYPE_MODULE(struct multiboot_tag *tag);
    void print_MULTIBOOT_TAG_TYPE_BASIC_MEMINFO(struct multiboot_tag *tag);
    void print_MULTIBOOT_TAG_TYPE_BOOTDEV(struct multiboot_tag *tag);
    void print_MULTIBOOT_TAG_TYPE_MMAP(struct multiboot_tag *tag);
    void print_MULTIBOOT_TAG_TYPE_ELF_SECTIONS(struct multiboot_tag *tag);
    void print_MULTIBOOT_TAG_TYPE_APM(struct multiboot_tag *tag);
    void print_MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR(struct multiboot_tag *tag);
    // 获取 e820 信息
    int get_e820(e820map_t &_e820map);
};

#endif /* _MULTIBOOT2_H_ */
