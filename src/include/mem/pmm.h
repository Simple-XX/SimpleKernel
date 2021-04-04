
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// pmm.h for Simple-XX/SimpleKernel.

#ifndef _PMM_H_
#define _PMM_H_

#include "stddef.h"
#include "stdint.h"
#include "io.h"
#include "multiboot2.h"
#include "e820.h"
#include "common.h"
#include "firstfit.h"

extern MULTIBOOT2::multiboot_memory_map_entry_t *mmap_entries;
extern MULTIBOOT2::multiboot_mmap_tag_t *        mmap_tag;

class PMM {
private:
    static IO io;
    // 可用内存的物理页数组
    static physical_pages_t phy_pages;

    // 管理算法的名称
    const char *name;
    FIRSTFIT    manage;

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
