
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// dtb.cpp for Simple-XX/SimpleKernel.

#include "stdint.h"
#include "stdio.h"
#include "endian.h"
#include "assert.h"
#include "common.h"
#include "pmm.h"
#include "dtb.h"

namespace DTB {
    // TODO: 完善
    // 现在没有考虑父节点等问题
    void dtb_iter(dtb_iter_fun_t _fun, void *_data) {
        // 匹配魔数
        assert(be32toh(boot_info_addr[0]) == FDT_MAGIC);
        // 匹配版本
        assert(be32toh(boot_info_addr[5]) == FDT_VERSION);
        // 设置 dtb 总大小
        boot_info_size = be32toh(boot_info_addr[1]);
        dtb_iter_t iter;
        // 指向数据区
        iter.offset = be32toh(boot_info_addr[2]);
        // 循环指针
        const uint32_t *pos =
            (uint32_t *)(((uint8_t *)boot_info_addr) + iter.offset);
        // 节点 tag
        iter.tag = be32toh(*pos);
        while (1) {
            switch (iter.tag) {
                // 新建节点
                case FDT_BEGIN_NODE: {
                    iter.node_name = (char *)(pos + 1);
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
                    iter.prop_name =
                        (char *)(((uint8_t *)boot_info_addr) +
                                 be32toh(boot_info_addr[3]) + be32toh(pos[2]));
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
            iter.offset = pos - boot_info_addr;
        }
        return;
    }

    // TODO: 完善
    bool get_memory(dtb_iter_t *_iter, void *_data) {
        // 找到 memory 属性节点
        if (strncmp((char *)_iter->node_name, "memory", sizeof("memory") - 1) ==
            0) {
            // 找到地址信息
            if (strcmp((char *)_iter->prop_name, "reg") == 0) {
                PMM::phy_mem_t *mem = (PMM::phy_mem_t *)_data;
                // 保存
                mem->addr = (void *)((uintptr_t)be32toh(_iter->prop_addr[0]) +
                                     (uintptr_t)be32toh(_iter->prop_addr[1]));
                mem->len =
                    be32toh(_iter->prop_addr[2]) + be32toh(_iter->prop_addr[3]);
                return true;
            }
        }
        return false;
    }

};

uint32_t *boot_info_addr = nullptr;
uint32_t  dtb_init_hart  = 0;
uint32_t  boot_info_size = 0;
