
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// pmm.h for Simple-XX/SimpleKernel.

#ifndef _PMM_H_
#define _PMM_H_

#include "stddef.h"
#include "stdint.h"
#include "multiboot2.h"
#include "e820.h"
#include "firstfit.h"

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
extern "C" uint8_t kernel_start[];
extern "C" uint8_t kernel_text_start[];
extern "C" uint8_t kernel_text_end[];
extern "C" uint8_t kernel_data_start[];
extern "C" uint8_t kernel_data_end[];
extern "C" uint8_t kernel_end[];

static const uint32_t KERNEL_START_ADDR =
    reinterpret_cast<uint32_t>(kernel_start);
static const uint32_t KERNEL_TEXT_START_ADDR =
    reinterpret_cast<uint32_t>(kernel_text_start);
static const uint32_t KERNEL_TEXT_END_ADDR =
    reinterpret_cast<uint32_t>(kernel_text_end);
static const uint32_t KERNEL_DATA_START_ADDR =
    reinterpret_cast<uint32_t>(kernel_data_start);
static const uint32_t KERNEL_DATA_END_ADDR =
    reinterpret_cast<uint32_t>(kernel_data_end);
static const uint32_t KERNEL_END_ADDR = reinterpret_cast<uint32_t>(kernel_end);

// PAE 标志的处理
#ifdef CPU_PAE
#else
#endif

// PSE 标志的处理
#ifdef CPU_PSE
// 页大小 4MB
static constexpr const uint32_t PMM_PAGE_SIZE = 0x400000;
#else
// 页大小 4KB
static constexpr const uint32_t PMM_PAGE_SIZE = 0x1000;
#endif

// 物理内存大小 2GB
static constexpr const uint32_t PMM_MAX_SIZE = 0x80000000;

// 物理页数量 131072,
// 0x20000，除去外设映射，实际可用物理页数量为 159 + 130800 =
// 130959 (这是物理内存为 512MB 的情况)
static constexpr const uint32_t PMM_PAGE_MAX_SIZE =
    PMM_MAX_SIZE / PMM_PAGE_SIZE;

// 页掩码，用于 4KB 对齐
static constexpr const uint32_t PMM_PAGE_MASK = 0xFFFFF000;
// 内核的偏移地址
static constexpr const uint32_t KERNEL_BASE = 0x0;
// 内核占用大小 8MB
static constexpr const uint32_t KERNEL_SIZE = 0x800000;
// 映射内核需要的页数
static constexpr const uint32_t PMM_PAGES_KERNEL = KERNEL_SIZE / PMM_PAGE_SIZE;

// 对齐 向上取整
#define ALIGN4K(x)                                                             \
    (((x) % PMM_PAGE_SIZE == 0) ? (x)                                          \
                                : (((x) + PMM_PAGE_SIZE - 1) & PMM_PAGE_MASK))

extern multiboot_memory_map_entry_t *mmap_entries;
extern multiboot_mmap_tag_t *        mmap_tag;

// 物理页结构体
typedef struct physical_page {
    // 起始地址
    uint8_t *addr;
    // 该页被引用次数，-1 表示此页内存被保留，禁止使用
    int32_t ref;
} physical_page_t;

// 可用内存的物理页数组
extern physical_page_t phy_pages[PMM_PAGE_MAX_SIZE];

class PMM {
private:
    // 4k 对齐的内核开始与结束地址
    uint8_t *kernel_start_align4k;
    uint8_t *kernel_end_align4k;
    // 管理算法的名称
    const char *name;
    FIRSTFIT    ff;

protected:
public:
    PMM(void);
    ~PMM(void);
    // 初始化内存管理
    int32_t init(void);
    // 从 GRUB 读取物理内存信息
    void get_ram_info(e820map_t *e820map);
    // 物理内存管理初始化 参数为实际可用物理页数量
    void mamage_init(uint32_t pages);
    // 请求指定数量物理页
    void *alloc_page(uint32_t pages);
    // 释放内存页
    void free_page(void *addr, uint32_t pages);
    // 获取空闲内存页数量
    uint32_t free_pages_count(void);
};

#endif /* _PMM_H_ */
