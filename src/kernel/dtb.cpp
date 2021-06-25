
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

dtb_prop_node_t::dtb_prop_node_t(mystl::string &_name) : name(_name) {
    return;
}

dtb_prop_node_t::~dtb_prop_node_t(void) {
    return;
}

void dtb_prop_node_t::add_prop(mystl::string _name, uint8_t *_val,
                               uint32_t _len) {
    (void)_name;
    (void)_val;
    (void)_len;
    // TODO
    // props[_name] = mystl::vector<uint8_t>(_val, _val + _len);
    return;
}

void dtb_prop_node_t::add_child(dtb_prop_node_t *_child) {
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

char *DTB::get_string(uint64_t _off) {
    return (char *)string_addr + _off;
}

// TODO: 子节点处理
mystl::pair<dtb_prop_node_t *, uint8_t *> DTB::get_node(uint8_t *_pos) {
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
    dtb_prop_node_t *node = new dtb_prop_node_t(name);
    // 处理属性
    tag = be32toh(*(uint32_t *)_pos);
    while (tag == FDT_PROP) {
        fdt_property_t *prop = (fdt_property_t *)_pos;
// #define DEBUG
#ifdef DEBUG
        printf("FDT_PROP, len: 0x%X, name: %s, value: ", be32toh(prop->len),
               get_string(be32toh(prop->nameoff)));
        for (uint32_t i = 0; i < be32toh(prop->len); i++) {
            printf("0x%X ", prop->data[i]);
        }
        printf("\n");
#undef DEBUG
#endif
        node->add_prop(get_string(be32toh(prop->nameoff)), prop->data,
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
    // 添加到向量中
    nodes.push_back(node);
    return mystl::pair<dtb_prop_node_t *, uint8_t *>(node, _pos);
}

const mystl::vector<dtb_prop_node_t *> DTB::find(mystl::string _name) {
    mystl::vector<dtb_prop_node_t *> res;
    for (auto i : nodes) {
        // TODO: 这里由于 libcxx 不支持使用 map["key"] 的形式，暂时使用 name
        // 比较，需要优化掉
        if (i->name == _name) {
            res.push_back(i);
        }
    }
    return res;
}

void dtb_preinit(uint32_t, uint64_t _addr) {
    dtb_addr = _addr;
    return;
}
