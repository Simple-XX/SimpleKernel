
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// dtb.cpp for Simple-XX/SimpleKernel.

#include "stdint.h"
#include "stdio.h"
#include "endian.h"
#include "assert.h"
#include "common.h"
#include "dtb.h"

namespace DTB {
    uint32_t *dtb_addr      = nullptr;
    uint32_t  dtb_init_hart = 0;
    uint32_t  dtb_size      = 0;

    // TODO: 完善
    // 现在没有考虑父节点等问题
    void dtb_iter(dtb_iter_fun_t _fun, void *_data) {
        // 匹配魔数
        assert(be32toh(dtb_addr[0]) == FDT_MAGIC);
        // 匹配版本
        assert(be32toh(dtb_addr[5]) == FDT_VERSION);
        // 设置 dtb 总大小
        dtb_size = be32toh(dtb_addr[1]);
        dtb_iter_t iter;
        // 指向数据区
        iter.offset = be32toh(dtb_addr[2]);
        // 循环指针
        const uint32_t *pos = (uint32_t *)(((uint8_t *)dtb_addr) + iter.offset);
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
                        (char *)(((uint8_t *)dtb_addr) + be32toh(dtb_addr[3]) +
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
            iter.offset = pos - dtb_addr;
        }
        return;
    }
};
