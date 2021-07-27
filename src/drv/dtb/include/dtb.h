
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// dtb.h for Simple-XX/SimpleKernel.

#ifndef _DTB_H_
#define _DTB_H_

#include "stdint.h"
#include "stdbool.h"

namespace DTB {
    // devicetree-specification-v0.3.pdf#5.1
    static constexpr const uint32_t FDT_MAGIC   = 0xD00DFEED;
    static constexpr const uint32_t FDT_VERSION = 0x11;
    struct fdt_header_t {
        // 魔数，0xD00DFEED
        uint32_t magic;
        // 整个 fdt 结构的大小
        uint32_t totalsize;
        // 数据区相对 header 地址的偏移
        uint32_t off_dt_struct;
        // 字符串区相对 header 地址的偏移
        uint32_t off_dt_strings;
        // 内存保留相对 header 地址的偏移
        uint32_t off_mem_rsvmap;
        // fdt 版本，17
        uint32_t version;
        // 最新的兼容版本
        uint32_t last_comp_version;
        // 启动 cpu 的 id
        uint32_t boot_cpuid_phys;
        // 字符串区长度
        uint32_t size_dt_strings;
        // 数据区长度
        uint32_t size_dt_struct;
    };

    struct fdt_property_t {
        uint32_t tag;
        // 表示 property value 的长度，byte 为单位
        uint32_t len;
        // property 的名称存放在 string block 区域，nameoff 表示其在 string
        // block 的偏移
        uint32_t nameoff;
        // property value 值，作为额外数据以'\0'结尾的字符串形式存储 structure
        // block, 32 - bits对齐，不够的位用 0x0 补齐
        uint32_t data[0];
    };

    // devicetree-specification-v0.3.pdf#5.4
    // node 开始标记
    static constexpr const uint32_t FDT_BEGIN_NODE = 0x1;
    // node 结束标记
    static constexpr const uint32_t FDT_END_NODE = 0x2;
    // 属性开始标记
    static constexpr const uint32_t FDT_PROP = 0x3;
    static constexpr const uint32_t FDT_NOP  = 0x4;
    // 数据区结束标记
    static constexpr const uint32_t FDT_END = 0x9;

    // 迭代变量
    struct dtb_iter_data_t {
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

    // 迭代函数
    typedef bool (*dtb_iter_fun_t)(dtb_iter_data_t *_iter_data, void *_data);
    // 迭代器
    void dtb_iter(dtb_iter_fun_t _fun, void *_data);
    // 保存 sbi 传递的启动核
    extern "C" size_t dtb_init_hart;
};

#endif /* _DTB_H_ */
