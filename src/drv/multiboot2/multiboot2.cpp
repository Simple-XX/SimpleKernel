
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// multiboot2.cpp for Simple-XX/SimpleKernel.

#include "assert.h"
#include "stdio.h"
#include "boot_info.hpp"
#include "multiboot2.h"
#include "common.h"

// 定义
namespace BOOT_INFO {
    // 地址
    void *boot_info_addr;
    // 长度
    size_t boot_info_size;
}

namespace MULTIBOOT2 {
    // 魔数
    uint32_t multiboot2_magic = 0;
    // TODO: 优化
    void multiboot2_iter(multiboot2_iter_fun_t _fun, void *_data) {
        void *addr = (uint32_t *)BOOT_INFO::boot_info_addr;
        // 判断魔数是否正确
        assert(multiboot2_magic == MULTIBOOT2_BOOTLOADER_MAGIC);
        assert((reinterpret_cast<uintptr_t>(addr) & 7) == 0);
        // addr+0 保存大小
        BOOT_INFO::boot_info_size = *(uint32_t *)addr;
        // 下一字节开始为 tag 信息
        void *tag_addr =
            reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(addr) + 8);
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
