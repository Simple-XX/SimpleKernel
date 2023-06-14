
/**
 * @file multiboot2.cpp
 * @brief multiboot2 解析实现
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

#include "multiboot2.h"
#include "boot_info.h"
#include "cassert"
#include "common.h"
#include "cstdio"
#include "resource.h"

MULTIBOOT2& MULTIBOOT2::get_instance(void) {
    /// 定义全局 MULTIBOOT2 对象
    static MULTIBOOT2 multiboot2;
    return multiboot2;
}

bool MULTIBOOT2::multiboot2_init(void) {
    uintptr_t addr = BOOT_INFO::boot_info_addr;
    // 判断魔数是否正确
    assert(BOOT_INFO::multiboot2_magic == MULTIBOOT2_BOOTLOADER_MAGIC);
    assert((reinterpret_cast<uintptr_t>(addr) & 7) == 0);
    // addr+0 保存大小
    BOOT_INFO::boot_info_size = *(uint32_t*)addr;
    return true;
}

/// @todo 优化
void MULTIBOOT2::multiboot2_iter(bool (*_fun)(const iter_data_t*, void*),
                                 void* _data) {
    uintptr_t    addr = BOOT_INFO::boot_info_addr;
    // 下一字节开始为 tag 信息
    iter_data_t* tag  = (iter_data_t*)(addr + 8);
    for (; tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (iter_data_t *)((uint8_t *)tag + COMMON::K_ALIGN(tag->size, 8))) {
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
bool MULTIBOOT2::get_memory(const iter_data_t* _iter_data, void* _data) {
    if (_iter_data->type != MULTIBOOT2::MULTIBOOT_TAG_TYPE_MMAP) {
        return false;
    }
    resource_t* resource  = (resource_t*)_data;
    resource->type       |= resource_t::MEM;
    resource->name        = (char*)"available phy memory";
    resource->mem.addr    = 0x0;
    resource->mem.len     = 0;
    MULTIBOOT2::multiboot_mmap_entry_t* mmap
      = ((MULTIBOOT2::multiboot_tag_mmap_t*)_iter_data)->entries;
    for (; (uint8_t*)mmap < (uint8_t*)_iter_data + _iter_data->size;
         mmap
         = (MULTIBOOT2::
              multiboot_mmap_entry_t*)((uint8_t*)mmap
                                       + ((MULTIBOOT2::multiboot_tag_mmap_t*)
                                            _iter_data)
                                           ->entry_size)) {
        // 如果是可用内存或地址小于 1M
        // 这里将 0~1M 的空间全部算为可用，在 c++ 库可用后进行优化
        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE
            || mmap->addr < 1 * COMMON::MB) {
            // 长度+
            resource->mem.len += mmap->len;
        }
    }
    return true;
}

namespace BOOT_INFO {
// 地址
uintptr_t boot_info_addr;
// 长度
size_t    boot_info_size;
// 魔数
uint32_t  multiboot2_magic;

bool      inited = false;

bool      init(void) {
    auto res = MULTIBOOT2::get_instance().multiboot2_init();
    if (inited == false) {
        inited = true;
        info("BOOT_INFO init.\n");
    }
    else {
        info("BOOT_INFO reinit.\n");
    }
    return res;
}

resource_t get_memory(void) {
    resource_t resource;
    MULTIBOOT2::get_instance().multiboot2_iter(MULTIBOOT2::get_memory,
                                               &resource);
    return resource;
}
};    // namespace BOOT_INFO
