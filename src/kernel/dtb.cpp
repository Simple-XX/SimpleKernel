
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

// TODO: 父节点的属性需要拷贝给子节点
void dtb_prop_node_t::add_prop(mystl::string _name, uint32_t *_addr,
                               uint32_t _len) {

    // TODO: 只处理了标准属性
    // 根据字段设置
    if (_name == mystl::string(standard_props_t::COMPATIBLE)) {
        standard.compatible = mystl::string((char *)_addr);
// #define DEBUG
#ifdef DEBUG
        printf("standard.compatible: %s\n", standard.compatible.c_str());
#endif
    }
    if (_name == mystl::string(standard_props_t::MODEL)) {
        standard.model = mystl::string((char *)_addr);
#ifdef DEBUG
        printf("standard.model: %s\n", standard.model.c_str());
#endif
    }
    if (_name == mystl::string(standard_props_t::PHANDLE)) {
        standard.phandle = be32toh(*_addr);
    }
    if (_name == mystl::string(standard_props_t::STATUS)) {
        standard.status = mystl::string((char *)_addr);
#ifdef DEBUG
        printf("standard.status: %s\n", standard.status.c_str());
#endif
    }
    if (_name == mystl::string(standard_props_t::ADDR_CELLS)) {
        standard.addr_cells = be32toh(*_addr);
    }
    if (_name == mystl::string(standard_props_t::SIZE_CELLS)) {
        standard.size_cells = be32toh(*_addr);
    }
    if (_name == mystl::string(standard_props_t::REG)) {
#ifdef DEBUG
        printf("reg: ");
#endif
        for (uint32_t i = 0; i < _len; i++) {
            standard.reg.push_back(be32toh(*(_addr + i)));
#ifdef DEBUG
            printf("0x%X ", standard.reg.at(i));
#endif
        }
#ifdef DEBUG
        printf("\n");
#endif
    }
    if (_name == mystl::string(standard_props_t::VIRTUAL_REG)) {
        standard.virt_reg = *_addr;
    }
    if (_name == mystl::string(standard_props_t::RANGES)) {
        for (uint32_t i = 0; i < _len; i++) {
            standard.ranges.push_back(be32toh(*(_addr + i)));
        }
    }
    if (_name == mystl::string(standard_props_t::DMA_RANGES)) {
        for (uint32_t i = 0; i < _len; i++) {
            standard.dma_ranges.push_back(be32toh(*(_addr + i)));
        }
    }
    // TODO: 设备中断只处理了一项
    if (_name == mystl::string(interrupt_device_props_t::INTERRUPTS)) {
        for (uint32_t i = 0; i < _len; i++) {
            interrupt_device.interrupts = be32toh(*_addr);
#ifdef DEBUG
            printf("interrupt_device.interrupts: 0x%X\n",
                   interrupt_device.interrupts);
#endif
        }
    }

#ifdef DEBUG
#undef DEBUG
#endif
    return;
}

void dtb_prop_node_t::add_child(dtb_prop_node_t *_child) {
    children.push_back(_child);
    return;
}

void *dtb_addr;

DTB::DTB(void) : addr(dtb_addr) {
    fdt_header_t header = *((fdt_header_t *)addr);
    // dtb 为大端，可能需要转换
    assert(be32toh(header.version) == 0x11);
    assert(header.magic == be32toh(FDT_MAGIC));
    size         = be32toh(header.totalsize);
    data_addr    = (ptrdiff_t)addr + be32toh(header.off_dt_struct);
    data_size    = be32toh(header.size_dt_struct);
    string_addr  = (ptrdiff_t)addr + be32toh(header.off_dt_strings);
    string_size  = be32toh(header.size_dt_strings);
    reserve_addr = (ptrdiff_t)addr + be32toh(header.off_mem_rsvmap);
    // 遍历保留区
    // devicetree-specification-v0.3.pdf#5.3.2
    fdt_reserve_entry_t *tmp1 = ((fdt_reserve_entry_t *)reserve_addr);
    fdt_reserve_entry_t  tmp2;
    while (1) {
        // 设置数据
        tmp2.address = be32toh(tmp1->address);
        tmp2.size    = be32toh(tmp1->size);
        // 如果为零则结束
        if (tmp2.size == 0 && tmp2.address == 0) {
            break;
        }
        // 不为零则添加到向量中
        // 保留区信息
        reserve.push_back(tmp2);
        // 下一项
        tmp1 += 1;
    }
    // 遍历数据区
    get_node((uint8_t *)data_addr);
// #define DEBUG
#ifdef DEBUG
#undef DEBUG
#endif
    printf("dtb init.\n");
    return;
}

DTB::~DTB(void) {
    return;
}

char *DTB::get_string(uint64_t _off) {
    return (char *)string_addr + _off;
}

// 处理节点
void DTB::tmp(void) {
    return;
}

// TODO: 子节点处理
mystl::pair<dtb_prop_node_t *, uint8_t *> DTB::get_node(uint8_t *_pos) {
    printf("111\n");
    uint32_t tag = be32toh(*(uint32_t *)_pos);
    // 新的递归必然从 FDT_BEGIN_NODE 开始
    assert(tag == FDT_BEGIN_NODE);
    // 获取 node name
    node_begin_t *node_begin = (node_begin_t *)_pos;
    mystl::string name       = (char *)node_begin->name;
    // 如果字段为空
    if (node_begin->name[0] == 0) {
        // 跳过 tag
        _pos += 4;
        // 跳过 name 区域
        // 因为为空所以跳过 4bytes 即可
        _pos += 4;
    }
    // 不为空的话需要进行计算
    else {
        // 跳过 tag
        _pos += 4;
        // 跳过 name 区域
        // 长度为 name 长度 4bytes 对齐
        _pos += COMMON::ALIGN(strlen((char *)node_begin->name) + 1, 4);
    }
    // 新建节点
    dtb_prop_node_t *node = new dtb_prop_node_t(name);
    // 处理属性
    tag = be32toh(*(uint32_t *)_pos);
    // 当 tag 表示当前为属性时
    while (tag == FDT_PROP) {
        // 开始处理
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
        // 添加节点属性
        // 以 4bytes 为单位，所以要 /4
        node->add_prop(get_string(be32toh(prop->nameoff)), prop->data,
                       be32toh(prop->len) / 4);
        // 移动 pos
        // 跳过 fdt_property_t.tag
        _pos += 4;
        // 跳过 fdt_property_t.len
        _pos += 4;
        // 跳过 fdt_property_t.nameoff
        _pos += 4;
        // 跳过 fdt_property_t.data
        _pos += be32toh(prop->len);
        // 计算下一个 _pos
        uint64_t tmp = (uint64_t)_pos;
        // 对齐
        tmp  = COMMON::ALIGN(tmp, 4);
        _pos = (uint8_t *)tmp;
        // 更新 tag
        tag = be32toh(*(uint32_t *)_pos);
    }

    // 所有属性处理完毕后开始处理子节点
    while (tag == FDT_BEGIN_NODE) {
        // 进入子节点的递归处理
        auto c = get_node(_pos);
        // 将子节点 c.first 添加到当前节点的 children 向量中
        node->add_child(c.first);
        // 更新 _pos
        _pos = c.second;
        // 更新 tag
        tag = be32toh(*(uint32_t *)_pos);
    }
    // 跳过 ?
    _pos += 4;
    // 将当前节点添加到 nodes 向量中
    nodes.push_back(node);
    // 返回当前处理的 node 和所在 _pos
    return mystl::pair<dtb_prop_node_t *, uint8_t *>(node, _pos);
}

const mystl::vector<dtb_prop_node_t *> DTB::find(mystl::string _name) {
    mystl::vector<dtb_prop_node_t *> res;
    // 遍历所有节点
    for (auto i : nodes) {
        // 找到属性中 kv 为 compatible:_name 的节点
        if (i->standard.compatible == _name) {
            // 添加到返回向量中
            res.push_back(i);
        }
    }
    return res;
}

void dtb_preinit(uint32_t, void *_addr) {
    dtb_addr = _addr;
    return;
}
