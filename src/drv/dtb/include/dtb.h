
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on https://github.com/brenns10/sos
// dtb.h for Simple-XX/SimpleKernel.

#ifndef _DTB_H_
#define _DTB_H_

#include "stdint.h"
#include "stdbool.h"
#include "boot_info.h"
#include "resource.h"
#include "endian.h"
#include "iostream"

// See devicetree-specification-v0.3.pdf
// https://e-mailky.github.io/2016-12-06-dts-introduce
// https://e-mailky.github.io/2019-01-14-dts-1
// https://e-mailky.github.io/2019-01-14-dts-2
// https://e-mailky.github.io/2019-01-14-dts-3

class DTB {
private:
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

    // devicetree-specification-v0.3.pdf#5.1
    static constexpr const uint32_t FDT_MAGIC   = 0xD00DFEED;
    static constexpr const uint32_t FDT_VERSION = 0x11;
    struct fdt_header_t {
        // devicetree-specification-v0.3.pdf#5.1
        static constexpr const uint32_t FDT_MAGIC   = 0xD00DFEED;
        static constexpr const uint32_t FDT_VERSION = 0x11;
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
        // 从地址构造
        fdt_header_t(const void *_addr);
        ~fdt_header_t(void);
    };

    // 内存保留区信息
    // devicetree-specification-v0.3.pdf#5.3.2
    struct fdt_reserve_entry_t {
        // 地址
        uint32_t addr_be;
        uint32_t addr_le;
        // 长度
        uint32_t size_be;
        uint32_t size_le;
    };
    // 属性节点格式，没有使用，仅用于参考
    struct fdt_property_t {
        // 第一个字节表示 type
        uint32_t tag;
        // 表示 property value 的长度，byte 为单位
        uint32_t len;
        // property 的名称存放在 string block 区域，nameoff 表示其在 string
        // block 的偏移
        uint32_t nameoff;
        // property value值，作为额外数据以'\0'结尾的字符串形式存储 structure
        // block, 32 - bits对齐，不够的位用 0x0 补齐
        uint32_t data[0];
    };

    // 路径最大深度
    static constexpr const size_t MAX_DEPTH = 16;
    // 最大节点数
    static constexpr const size_t MAX_NODES = 128;
    // 最大属性数
    static constexpr const size_t PROP_MAX = 16;
    // dtb 信息
    struct dtb_info_t {
        // dtb 头
        fdt_header_t *header;
        // 保留区
        fdt_reserve_entry_t *reserved;
        // 数据区
        uintptr_t data;
        // 字符区
        uintptr_t str;
    };
    // 属性信息
    struct prop_t {
        // 属性名
        char *name;
        // 属性地址
        uintptr_t addr;
        // 属性长度
        size_t len;
    };
    // 路径
    struct path_t {
        // 当前路径
        char *path[MAX_DEPTH];
        // 长度
        size_t len;
        // 全部匹配
        bool operator==(const path_t *_path);
    };
    // 节点数据
    struct node_t {
        // 节点路径
        path_t path;
        // 节点地址
        uint32_t *addr;
        // 父节点
        node_t *parent;
        // 中断父节点
        node_t *interrupt_parent;
        // 1 cell == 4 bytes
        // 地址长度 单位为 bytes
        uint32_t address_cells;
        // 长度长度 单位为 bytes
        uint32_t size_cells;
        // 中断长度 单位为 bytes
        uint32_t interrupt_cells;
        uint32_t phandle;
        // 路径深度
        uint8_t depth;
        // 属性
        prop_t props[PROP_MAX];
        // 属性数
        size_t prop_count;
        // 节点数
        static size_t count;
        // 查找节点中的键值对
        bool find(const char *_prop_name, const char *_val);
    };

protected:
public:
    // 迭代变量
    struct iter_data_t {
        // 路径，不包括节点名
        path_t path;
        // 节点地址
        uint32_t *addr;
        // 节点类型
        uint32_t type;
        // 如果节点类型为 PROP， 保存节点属性名
        char *prop_name;
        // 如果节点类型为 PROP， 保存属性长度 单位为 byte
        uint32_t prop_len;
        // 如果节点类型为 PROP， 保存属性地址
        uint32_t *prop_addr;
        // 在 nodes 数组的下标
        uint8_t nodes_idx;
    };

    // 部分属性及格式
    // devicetree-specification-v0.3#2.3
    // devicetree-specification-v0.3#2.4.1
    // 格式
    enum dt_fmt_t {
        // 未知
        FMT_UNKNOWN = 0,
        // 空
        FMT_EMPTY,
        FMT_U32,
        FMT_U64,
        // 字符串
        FMT_STRING,
        FMT_PHANDLE,
        // 字符串列表
        FMT_STRINGLIST,
        FMT_REG,
        FMT_RANGES,
    };

    // 用于 get_fmt
    struct dt_prop_fmt_t {
        // 属性名
        char *prop_name;
        // 格式
        enum dt_fmt_t fmt;
    };
    // 用于 get_fmt
    // 格式信息请查看 devicetree-specification-v0.3#2.3,#2.4 等部分
    static constexpr const dt_prop_fmt_t props[] = {
        {.prop_name = (char *)"", .fmt = FMT_EMPTY},
        {.prop_name = (char *)"compatible", .fmt = FMT_STRINGLIST},
        {.prop_name = (char *)"model", .fmt = FMT_STRING},
        {.prop_name = (char *)"phandle", .fmt = FMT_U32},
        {.prop_name = (char *)"status", .fmt = FMT_STRING},
        {.prop_name = (char *)"#address-cells", .fmt = FMT_U32},
        {.prop_name = (char *)"#size-cells", .fmt = FMT_U32},
        {.prop_name = (char *)"#interrupt-cells", .fmt = FMT_U32},
        {.prop_name = (char *)"reg", .fmt = FMT_REG},
        {.prop_name = (char *)"virtual-reg", .fmt = FMT_U32},
        {.prop_name = (char *)"ranges", .fmt = FMT_RANGES},
        {.prop_name = (char *)"dma-ranges", .fmt = FMT_RANGES},
        {.prop_name = (char *)"name", .fmt = FMT_STRING},
        {.prop_name = (char *)"device_type", .fmt = FMT_STRING},
        {.prop_name = (char *)"interrupts", .fmt = FMT_U32},
        {.prop_name = (char *)"interrupt-parent", .fmt = FMT_PHANDLE},
        {.prop_name = (char *)"interrupt-controller", .fmt = FMT_EMPTY},
        {.prop_name = (char *)"value", .fmt = FMT_U32},
        {.prop_name = (char *)"offset", .fmt = FMT_U32},
        {.prop_name = (char *)"regmap", .fmt = FMT_U32},
    };
    // 查找 _prop_name 在 dt_fmt_t 的索引
    static dt_fmt_t get_fmt(const char *_prop_name);

    // phandles 与 node 的映射关系
    struct phandle_map_t {
        uint32_t phandle;
        node_t * node;
        // phandle 数量
        static size_t count;
    };

    // dtb 信息
    static dtb_info_t dtb_info;
    // 节点数组
    static node_t nodes[MAX_NODES];
    // phandle 数组
    static phandle_map_t phandle_map[MAX_NODES];
    // 输出 reserved 内存
    static void dtb_mem_reserved(void);
    // 迭代函数
    static void dtb_iter(uint8_t _cb_flags,
                         bool (*_cb)(const iter_data_t *, void *), void *_data,
                         uintptr_t _addr = dtb_info.data);
    // 查找 phandle 映射
    static node_t *get_phandle(uint32_t _phandle);
    // 初始化节点
    static bool dtb_init_cb(const iter_data_t *_iter, void *_data);
    // 初始化中断信息
    static bool dtb_init_interrupt_cb(const iter_data_t *_iter, void *_data);
    // 输出不定长度的数据
    static void print_attr_propenc(const iter_data_t *_iter, size_t *_cells,
                                   size_t _len);
    // 填充 resource
    // _resource: 被填充的
    // _node: src 节点，要使用 cells 等信息
    // _prop: 填充的数据
    static void fill_resource(resource_t *_resource, const node_t *_node,
                              const prop_t *_prop);
    // 在所有节点中寻找 _prop_name/_val 对符合的节点
    static node_t *find_node(const char *_prop_name, const char *_val);
    // 用于标记是否第一次 init
    static bool inited;

protected:
public:
    // 用于控制处理哪些属性
    static constexpr const uint8_t DT_ITER_BEGIN_NODE = 0x01;
    static constexpr const uint8_t DT_ITER_END_NODE   = 0x02;
    static constexpr const uint8_t DT_ITER_PROP       = 0x04;
    // 初始化
    static bool dtb_init(void);
    // 寻找属性-值向匹配的节点，返回其使用的资源
    // TODO: 只能返回一个资源，没法处理一个节点使用复数资源的情况
    static resource_t find(const char *_prop_name, const char *_val);
    // iter 输出
    friend std::ostream &operator<<(std::ostream &     _os,
                                    const iter_data_t &_iter);
    // 路径输出
    friend std::ostream &operator<<(std::ostream &_os, const path_t &_path);
};

namespace BOOT_INFO {
    // 保存 sbi 传递的启动核
    extern "C" size_t dtb_init_hart;
};

extern "C" size_t dtb_init_hart;

#endif /* _DTB_H_ */
