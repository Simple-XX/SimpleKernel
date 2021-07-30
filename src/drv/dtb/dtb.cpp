
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// dtb.cpp for Simple-XX/SimpleKernel.

#include "stdint.h"
#include "stdio.h"
#include "endian.h"
#include "assert.h"
#include "common.h"
#include "boot_info.h"
#include "resource.h"
#include "dtb.h"

// 地址
uintptr_t boot_info_addr;
// 长度
size_t BOOT_INFO::boot_info_size;
// 启动核
size_t dtb_init_hart;

struct BOOT_INFO::iter_data_t {
    // 节点标记
    uint32_t tag;
    // 节点名
    char *node_name;
    // 当前节点在 dtb 中的偏移
    uint32_t offset;
    // 如果 tag 为 FDT_PROP，保存属性名，否则为 nullptr
    char *prop_name;
    // 如果 tag 为 FDT_PROP，保存属性地址，否则为 nullptr
    uint32_t *prop_addr;
    // 如果 tag 为 FDT_PROP，保存属性长度，否则为 0
    uint32_t prop_len;
};

// TODO: 完善
// 现在没有考虑父节点等问题
void DTB::dtb_iter(BOOT_INFO::iter_fun_t _fun, void *_data) {
    const uint32_t *addr = (uint32_t *)boot_info_addr;
    // 匹配魔数
    assert(be32toh(addr[0]) == FDT_MAGIC);
    // 匹配版本
    assert(be32toh(addr[5]) == FDT_VERSION);
    // 设置 dtb 总大小
    BOOT_INFO::boot_info_size = be32toh(addr[1]);
    BOOT_INFO::iter_data_t iter;
    // 指向数据区
    iter.offset = be32toh(addr[2]);
    // 循环指针
    const uint32_t *pos = (uint32_t *)(((uint8_t *)addr) + iter.offset);
    // 节点 tag
    iter.tag = be32toh(*pos);
    while (1) {
        switch (iter.tag) {
            // 新建节点
            case FDT_BEGIN_NODE: {
                iter.node_name = (char *)(pos + 1);
                // 如果是第一个节点，节点名 “/” 会被省略，手动加上
                if (pos == (uint32_t *)(((uint8_t *)addr) + iter.offset)) {
                    iter.node_name = (char *)"/";
                }
                // 跳过 tag
                pos++;
                // 跳过 name
                pos += COMMON::ALIGN(strlen((char *)pos) + 1, 4) / 4;
                break;
            }
            // 节点结束
            case FDT_END_NODE: {
                // 跳过 tag
                pos++;
                break;
            }
            // 属性节点
            case FDT_PROP: {
                // 属性名
                iter.prop_name = (char *)(((uint8_t *)addr) + be32toh(addr[3]) +
                                          be32toh(pos[2]));
                // 属性地址
                // +3 分别是 tag，len，nameoff
                iter.prop_addr = (uint32_t *)(pos + 3);
                // 属性长度
                iter.prop_len = be32toh(pos[1]);
                if (_fun(&iter, _data) == true) {
                    return;
                }
                // 跳过 tag
                pos++;
                // 跳过 len
                pos++;
                // 跳过 nameoff
                pos++;
                // 跳过属性数据
                pos += COMMON::ALIGN(iter.prop_len, 4) / 4;
                break;
            }
            // 直接跳过
            case FDT_NOP: {
                pos++;
                break;
            }
            // dtb 结束，直接返回
            case FDT_END: {
                return;
            }
            // 不是以上类型则出错
            default: {
                assert(0);
                return;
            }
        }
        // 更新 tag
        iter.tag = be32toh(*pos);
        // 更新 offset
        iter.offset = pos - addr;
    }
    return;
}

// TODO: 完善
bool DTB::get_memory(BOOT_INFO::iter_data_t *_iter_data, void *_data) {
    // 找到 memory 属性节点
    if (strncmp((char *)_iter_data->node_name, "memory", strlen("memory")) ==
        0) {
        // 找到地址信息
        if (strcmp((char *)_iter_data->prop_name, "reg") == 0) {
            resource_t *resource = (resource_t *)_data;
            resource->type       = resource_t::MEM;
            resource->name       = (char *)"phy memory";
            resource->mem.addr = ((uintptr_t)be32toh(_iter_data->prop_addr[0]) +
                                  (uintptr_t)be32toh(_iter_data->prop_addr[1]));
            resource->mem.len  = be32toh(_iter_data->prop_addr[2]) +
                                be32toh(_iter_data->prop_addr[3]);
            return true;
        }
    }
    return false;
}

resource_t BOOT_INFO::get_memory(void) {
    resource_t resource;
    DTB::dtb_iter(DTB::get_memory, &resource);
    return resource;
}
