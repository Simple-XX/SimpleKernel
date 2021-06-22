
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// dtb.cpp for Simple-XX/SimpleKernel.

#include "string.h"
#include "stdio.h"
#include "assert.h"
#include "endian.h"
#include "dtb.h"
#include "iostream"
#include "common.h"

PROP_NODE::PROP_NODE(mystl::string &_name) : name(_name) {
    return;
}

PROP_NODE::~PROP_NODE(void) {
    return;
}

void PROP_NODE::add_prop(mystl::string _name, uint8_t *_val, uint32_t _len) {
    (void)_name;
    (void)_val;
    (void)_len;
    // TODO
    // props[_name] = mystl::vector<uint8_t>(_val, _val + _len);
    return;
}

void PROP_NODE::add_child(PROP_NODE *_child) {
    children.push_back(_child);
    return;
}

uint64_t dtb_addr;

DTB::DTB(void) : addr(dtb_addr) {
    assert(addr == 0x82200000);
    fdt_header_t header = *((fdt_header_t *)addr);
    // dtb 为大端，可能需要转换
    assert(be32toh(header.version) == 0x11);
    assert(header.magic == be32toh(FDT_MAGIC));
    size         = be32toh(header.totalsize);
    data_addr    = addr + be32toh(header.off_dt_struct);
    data_size    = be32toh(header.size_dt_struct);
    string_addr  = addr + be32toh(header.off_dt_strings);
    string_size  = be32toh(header.size_dt_strings);
    reserve_addr = addr + be32toh(header.off_mem_rsvmap);
    // 遍历保留区
    // devicetree-specification-v0.3.pdf#5.3.2
    fdt_reserve_entry_t *res = ((fdt_reserve_entry_t *)reserve_addr);
    fdt_reserve_entry_t  tmp;
    while (1) {
        tmp.address = be32toh(res->address);
        tmp.size    = be32toh(res->size);
        // 保留区信息
        reserve.push_back(tmp);
        res += 1;
        if (tmp.size == 0 && tmp.address == 0) {
            break;
        }
    }
    get_node((uint8_t *)data_addr);
    printf("dtb init.\n");
    return;
}

DTB::~DTB(void) {
    return;
}

char *DTB::get_name(uint64_t _off) {
    return (char *)string_addr + _off;
}

mystl::pair<PROP_NODE *, uint8_t *> DTB::get_node(uint8_t *_pos) {
    uint32_t tag = be32toh(*(uint32_t *)_pos);
    // 新的递归必然从 FDT_BEGIN_NODE 开始
    assert(tag == FDT_BEGIN_NODE);
    // 获取 node name
    node_begin_t *node_begin = (node_begin_t *)_pos;
    mystl::string name       = (char *)node_begin->name;
    // 如果字段为空
    if (node_begin->name[0] == 0) {
        _pos += 4;
        // 直接跳过
        _pos += 4;
    }
    // 不为空的话需要进行计算
    else {
        _pos += 4;
        _pos += COMMON::ALIGN(strlen((char *)node_begin->name) + 1, 4);
    }
    // 新建节点
    PROP_NODE *node = new PROP_NODE(name);
    // 处理属性
    tag = be32toh(*(uint32_t *)_pos);
    while (tag == FDT_PROP) {
        fdt_property_t *prop = (fdt_property_t *)_pos;
// #define DEBUG
#ifdef DEBUG
        printf("FDT_PROP, len: 0x%X, name: %s, value: ", be32toh(prop->len),
               get_name(be32toh(prop->nameoff)));
        for (uint32_t i = 0; i < be32toh(prop->len); i++) {
            printf("0x%X ", prop->data[i]);
        }
        printf("\n");
#undef DEBUG
#endif
        node->add_prop(get_name(be32toh(prop->nameoff)), prop->data,
                       be32toh(prop->len));
        // 移动 pos
        _pos += 4;
        _pos += 4;
        _pos += 4;
        _pos += be32toh(prop->len);
        uint64_t tmp = (uint64_t)_pos;
        tmp          = COMMON::ALIGN(tmp, 4);
        _pos         = (uint8_t *)tmp;
        tag          = be32toh(*(uint32_t *)_pos);
    }

    // 处理子节点
    tag = be32toh(*(uint32_t *)_pos);
    while (tag == FDT_BEGIN_NODE) {
        auto c = get_node(_pos);
        node->add_child(c.first);
        _pos = c.second;
        tag  = be32toh(*(uint32_t *)_pos);
    }
    _pos += 4;
    return mystl::pair<PROP_NODE *, uint8_t *>(node, _pos);
}

void dtb_init(uint32_t, uint64_t _addr) {
    dtb_addr = _addr;
    return;
}
