
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// multiboot2.cpp for Simple-XX/SimpleKernel.

#include "assert.h"
#include "stdio.h"
#include "common.h"
#include "multiboot2.h"
#include "boot_info.h"
#include "multiboot2.h"
#include "resource.h"
#include "common.h"
#include "pmm.h"

// 地址
uintptr_t boot_info_addr;
// 长度
size_t BOOT_INFO::boot_info_size;
// 魔数
uint32_t multiboot2_magic;

struct BOOT_INFO::iter_data_t {
    // 与 multiboot_tag_t 相同
    uint32_t type;
    uint32_t size;
};

// TODO: 优化
void MULTIBOOT2::multiboot2_iter(BOOT_INFO::iter_fun_t _fun, void *_data) {
    void *addr = (void *)boot_info_addr;
    // 判断魔数是否正确
    assert(multiboot2_magic == MULTIBOOT2_BOOTLOADER_MAGIC);
    assert((reinterpret_cast<uintptr_t>(addr) & 7) == 0);
    // addr+0 保存大小
    BOOT_INFO::boot_info_size = *(uint32_t *)addr;
    // 下一字节开始为 tag 信息
    void *tag_addr =
        reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(addr) + 8);
    BOOT_INFO::iter_data_t *tag = (BOOT_INFO::iter_data_t *)tag_addr;
    for (tag = (BOOT_INFO::iter_data_t *)tag_addr;
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (BOOT_INFO::iter_data_t *)((uint8_t *)tag +
                                          COMMON::ALIGN(tag->size, 8))) {
        if (_fun(tag, _data) == true) {
            return;
        }
    }
    return;
}

// 读取 grub2 传递的物理内存信息，保存到 e820map_t 结构体中
// 一般而言是这样的
// 地址(长度) 类型
// 0x00(0x9F000) 0x1
// 0x9F000(0x1000) 0x2
// 0xE8000(0x18000) 0x2
// 0x100000(0x7EF0000) 0x1
// 0x7FF0000(0x10000) 0x3
// 0xFFFC0000(0x40000) 0x2
bool MULTIBOOT2::get_memory(BOOT_INFO::iter_data_t *_iter_data, void *_data) {
    if (_iter_data->type != MULTIBOOT2::MULTIBOOT_TAG_TYPE_MMAP) {
        return false;
    }
    resource_t *resource = (resource_t *)_data;
    resource->type       = resource_t::MEM;
    resource->name       = (char *)"available phy memory";
    resource->mem.addr   = 0x0;
    resource->mem.len    = 0;
    MULTIBOOT2::multiboot_mmap_entry_t *mmap =
        ((MULTIBOOT2::multiboot_tag_mmap_t *)_iter_data)->entries;
    for (; (uint8_t *)mmap < (uint8_t *)_iter_data + _iter_data->size;
         mmap = (MULTIBOOT2::multiboot_mmap_entry_t
                     *)((uint8_t *)mmap +
                        ((MULTIBOOT2::multiboot_tag_mmap_t *)_iter_data)
                            ->entry_size)) {
        // 如果是可用内存或地址小于 1M
        // 这里将 0~1M 的空间全部算为可用，在 c++ 库可用后进行优化
        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE ||
            mmap->addr < 1 * COMMON::MB) {
            // 长度+
            resource->mem.len += mmap->len;
        }
    }
    return true;
}

resource_t BOOT_INFO::get_memory(void) {
    resource_t resource;
    MULTIBOOT2::multiboot2_iter(MULTIBOOT2::get_memory, &resource);
    return resource;
}
