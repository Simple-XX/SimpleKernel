
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// multiboot2.cpp for Simple-XX/SimpleKernel.

#include "assert.h"
#include "stdio.h"
#include "multiboot2.h"
#include "common.h"
#include "pmm.h"
#include "e820.h"

namespace MULTIBOOT2 {
    // 保存完整的内存信息
    static e820map_t e820map;
    // 读取 grub2 传递的物理内存信息，保存到 e820map_t 结构体中
    // 一般而言是这样的
    // 地址(长度) 类型
    // 0x00(0x9F000) 0x1
    // 0x9F000(0x1000) 0x2
    // 0xE8000(0x18000) 0x2
    // 0x100000(0x7EF0000) 0x1
    // 0x7FF0000(0x10000) 0x3
    // 0xFFFC0000(0x40000) 0x2
    bool get_memory(MULTIBOOT2::multiboot_tag_t *_tag, void *_data) {
        if (_tag->type != MULTIBOOT2::MULTIBOOT_TAG_TYPE_MMAP) {
            return false;
        }
        PMM::phy_mem_t *data = (PMM::phy_mem_t *)_data;
        data->addr           = nullptr;
        MULTIBOOT2::multiboot_mmap_entry_t *mmap =
            ((MULTIBOOT2::multiboot_tag_mmap_t *)_tag)->entries;
        for (; (uint8_t *)mmap < (uint8_t *)_tag + _tag->size;
             mmap = (MULTIBOOT2::multiboot_mmap_entry_t
                         *)((uint8_t *)mmap +
                            ((MULTIBOOT2::multiboot_tag_mmap_t *)_tag)
                                ->entry_size)) {
            e820map.map[e820map.nr_map].addr   = (uintptr_t)mmap->addr;
            e820map.map[e820map.nr_map].length = (uintptr_t)mmap->len;
            e820map.map[e820map.nr_map].type   = mmap->type;
            // 最后更新 _data 数据
            data->len += (uintptr_t)mmap->len;
            e820map.nr_map++;
        }
        return true;
    }

    // TODO: 优化
    void multiboot2_iter(multiboot2_iter_fun_t _fun, void *_data) {
        // 判断魔数是否正确
        assert(multiboot2_magic == MULTIBOOT2_BOOTLOADER_MAGIC);
        assert((reinterpret_cast<uintptr_t>(boot_info_addr) & 7) == 0);
        // boot_info_addr+0 保存大小
        boot_info_size = *(uint32_t *)boot_info_addr;
        // 下一字节开始为 tag 信息
        void *tag_addr = reinterpret_cast<void *>(
            reinterpret_cast<uintptr_t>(boot_info_addr) + 8);
        multiboot_tag_t *tag = (multiboot_tag_t *)tag_addr;
        for (tag = (multiboot_tag_t *)tag_addr;
             tag->type != MULTIBOOT_TAG_TYPE_END;
             tag = (multiboot_tag_t *)((uint8_t *)tag +
                                       COMMON::ALIGN(tag->size, 8))) {
            if (_fun(tag, _data) == true) {
                return;
            }
        }
        return;
    }
};

// 地址
void *boot_info_addr = nullptr;
// 魔数
uint32_t multiboot2_magic = 0;
uint32_t boot_info_size   = 0;
