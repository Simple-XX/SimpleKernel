
/**
 * @file multiboot2.h
 * @brief multiboot2 解析
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-09-18<td>digmouse233<td>迁移到 doxygen
 * </table>
 */

#ifndef _MULTIBOOT2_H_
#define _MULTIBOOT2_H_

#include "stdint.h"
#include "stdbool.h"
#include "boot_info.h"

/// @see Multiboot2 Specification version 2.0.pdf
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

/**
 * @brief MULTIBOOT2 接口抽象
 */
class MULTIBOOT2 {
private:
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

    struct multiboot_header_t {
        // Must be MULTIBOOT_MAGIC - see above.
        uint32_t magic;
        // ISA
        uint32_t architecture;
        // Total header length.
        uint32_t header_length;
        // The above fields plus this one must equal 0 mod 2^32.
        uint32_t checksum;
    };

    struct multiboot_header_tag_t {
        uint16_t type;
        uint16_t flags;
        uint32_t size;
    };

    struct multiboot_header_tag_information_request_t : multiboot_header_tag_t {
        uint32_t requests[0];
    };

    struct multiboot_header_tag_address_t : multiboot_header_tag_t {
        uint32_t header_addr;
        uint32_t load_addr;
        uint32_t load_end_addr;
        uint32_t bss_end_addr;
    };

    struct multiboot_header_tag_entry_address_t : multiboot_header_tag_t {
        uint32_t entry_addr;
    };

    struct multiboot_header_tag_console_flags_t : multiboot_header_tag_t {
        uint32_t console_flags;
    };

    struct multiboot_header_tag_framebuffer_t : multiboot_header_tag_t {
        uint32_t width;
        uint32_t height;
        uint32_t depth;
    };

    struct multiboot_header_tag_module_align_t : multiboot_header_tag_t {
        ;
    };

    struct multiboot_header_tag_relocatable_t : multiboot_header_tag_t {
        uint32_t min_addr;
        uint32_t max_addr;
        uint32_t align;
        uint32_t preference;
    };

    struct multiboot_color_t {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    };

    static constexpr const uint32_t MULTIBOOT_MEMORY_AVAILABLE        = 1;
    static constexpr const uint32_t MULTIBOOT_MEMORY_RESERVED         = 2;
    static constexpr const uint32_t MULTIBOOT_MEMORY_ACPI_RECLAIMABLE = 3;
    static constexpr const uint32_t MULTIBOOT_MEMORY_NVS              = 4;
    static constexpr const uint32_t MULTIBOOT_MEMORY_BADRAM           = 5;
    struct multiboot_mmap_entry_t {
        uint64_t addr;
        uint64_t len;
        uint32_t type;
        uint32_t zero;
    };

    struct multiboot_tag_t {
        uint32_t type;
        uint32_t size;
    };

    struct multiboot_tag_string_t : multiboot_tag_t {
        char string[0];
    };

    struct multiboot_tag_module_t : multiboot_tag_t {
        uint32_t mod_start;
        uint32_t mod_end;
        char     cmdline[0];
    };

    struct multiboot_tag_basic_meminfo_t : multiboot_tag_t {
        uint32_t mem_lower;
        uint32_t mem_upper;
    };

    struct multiboot_tag_bootdev_t : multiboot_tag_t {
        uint32_t biosdev;
        uint32_t slice;
        uint32_t part;
    };

    struct multiboot_tag_mmap_t : multiboot_tag_t {
        uint32_t               entry_size;
        uint32_t               entry_version;
        multiboot_mmap_entry_t entries[0];
    };

    struct multiboot_vbe_info_block_t {
        uint8_t external_specification[512];
    };

    struct multiboot_vbe_mode_info_block_t {
        uint8_t external_specification[256];
    };

    struct multiboot_tag_vbe_t : multiboot_tag_t {
        uint16_t vbe_mode;
        uint16_t vbe_interface_seg;
        uint16_t vbe_interface_off;
        uint16_t vbe_interface_len;

        multiboot_vbe_info_block_t      vbe_control_info;
        multiboot_vbe_mode_info_block_t vbe_mode_info;
    };

    struct multiboot_tag_elf_sections_t : multiboot_tag_t {
        uint32_t num;
        uint32_t entsize;
        // 段字符串表索引
        uint32_t shndx;
        char     sections[0];
    };

    struct multiboot_tag_apm_t : multiboot_tag_t {
        uint16_t version;
        uint16_t cseg;
        uint32_t offset;
        uint16_t cseg_16;
        uint16_t dseg;
        uint16_t flags;
        uint16_t cseg_len;
        uint16_t cseg_16_len;
        uint16_t dseg_len;
    };

    struct multiboot_tag_efi32_t : multiboot_tag_t {
        uint32_t pointer;
    };

    struct multiboot_tag_efi64_t : multiboot_tag_t {
        uint64_t pointer;
    };

    struct multiboot_tag_smbios_t : multiboot_tag_t {
        uint8_t major;
        uint8_t minor;
        uint8_t reserved[6];
        uint8_t tables[0];
    };

    struct multiboot_tag_old_acpi_t : multiboot_tag_t {
        uint8_t rsdp[0];
    };

    struct multiboot_tag_new_acpi_t : multiboot_tag_t {
        uint8_t rsdp[0];
    };

    struct multiboot_tag_network_t : multiboot_tag_t {
        uint8_t dhcpack[0];
    };

    struct multiboot_tag_efi_mmap_t : multiboot_tag_t {
        uint32_t descr_size;
        uint32_t descr_vers;
        uint8_t  efi_mmap[0];
    };

    struct multiboot_tag_efi32_ih_t : multiboot_tag_t {
        uint32_t pointer;
    };

    struct multiboot_tag_efi64_ih_t : multiboot_tag_t {
        uint64_t pointer;
    };

    struct multiboot_tag_load_base_addr_t : multiboot_tag_t {
        uint32_t load_base_addr;
    };

    // 迭代变量
    // 与 multiboot_tag_t 相同
    struct iter_data_t {
        uint32_t type;
        uint32_t size;
    };

public:
    /**
     * @brief 获取单例
     * @return MULTIBOOT2&      静态对象
     */
    static MULTIBOOT2 &get_instance(void);

    /**
     * @brief 初始化
     * @return true            成功
     * @return false           失败
     */
    bool multiboot2_init(void);

    /**
     * @brief 迭代器
     * @param  _fun            迭代操作
     * @param  _data           数据
     */
    void multiboot2_iter(bool (*_fun)(const iter_data_t *, void *),
                         void *_data);

    /**
     * @brief 获取内存信息
     * @param  _iter_data      迭代变量
     * @param  _data           数据
     * @return true            成功
     * @return false           失败
     */
    static bool get_memory(const iter_data_t *_iter_data, void *_data);
};

namespace BOOT_INFO {
/// 魔数
extern "C" uint32_t multiboot2_magic;
}; // namespace BOOT_INFO

#endif /* _MULTIBOOT2_H_ */
