
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// dtb.h for Simple-XX/SimpleKernel.

#ifndef _DTB_H_
#define _DTB_H_

#include "stdint.h"
#include "stdbool.h"
#include "resource.h"
#include "endian.h"
#include "iostream"

// 保存 sbi 传递的启动核
extern "C" size_t dtb_init_hart;

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
    // 路径最大深度
    static constexpr const size_t MAX_DEPTH = 16;
    // 最大节点数
    static constexpr const size_t MAX_NODES = 128;
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
    // 节点数据
    struct node_t {
        // 节点名
        char *name;
        //
        uint32_t *tok;
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
        // 节点数
        static size_t count;
    };

    // phandles 与 node 的映射关系
    struct phandle_map_t {
        uint32_t phandle;
        node_t * node;
        // phandle 数量
        static size_t count;
    };

    // 路径
    struct path_t {
        // 当前路径
        char *path[MAX_DEPTH];
        // 长度
        size_t               len;
        friend std::ostream &operator<<(std::ostream &_os,
                                        const path_t &_path) {
            if (_path.len == 1) {
                _os << "/";
            }
            else {
                for (size_t i = 1; i < _path.len; i++) {
                    _os << "/";
                    _os << _path.path[i];
                }
            }
            return _os;
        }
    };

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

    struct dt_propenc_field_t {
        size_t cells;
        bool   phandle;
    };

    // 输出属性
    static void print_attr_propenc(const iter_data_t *_iter,
                                   dt_propenc_field_t fields[], size_t fcnt);

    // 部分属性及格式
    // devicetree-specification-v0.3#2.3
    // devicetree-specification-v0.3#2.4.1
    // 格式
    enum dt_fmt_t {
        FMT_UNKNOWN = 0,
        FMT_EMPTY,
        FMT_U32,
        FMT_U64,
        FMT_STRING,
        FMT_PHANDLE,
        FMT_STRINGLIST,
        FMT_REG,
        FMT_RANGES,
        FMT_INTERRUPTS,
    };

    struct dt_prop_fmt_t {
        // 属性名
        char *prop_name;
        // 格式
        enum dt_fmt_t fmt;
    };

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
        {.prop_name = (char *)"interrupts", .fmt = FMT_INTERRUPTS},
        {.prop_name = (char *)"interrupt-parent", .fmt = FMT_PHANDLE},
        {.prop_name = (char *)"interrupt-controller", .fmt = FMT_EMPTY},
        {.prop_name = (char *)"value", .fmt = FMT_U32},
        {.prop_name = (char *)"offset", .fmt = FMT_U32},
        {.prop_name = (char *)"regmap", .fmt = FMT_U32},

    };
    // 查找 _prop_name 在 dt_fmt_t 的索引
    static dt_fmt_t get_fmt(const char *_prop_name);

    // dtb 信息
    static dtb_info_t info;
    // 节点数组
    static node_t nodes[MAX_NODES];
    // phandle 数组
    static phandle_map_t phandle_map[MAX_NODES];
    // 输出 reserved 内存
    static void dtb_mem_reserved(void);
    // 迭代函数
    static void dtb_iter(uint8_t _cb_flags,
                         bool (*_cb)(const iter_data_t *, void *), void *_data);
    // 查找 phandle 映射
    static node_t *get_phandle(uint32_t _phandle);
    // 初始化节点
    static bool dtb_init_cb(const iter_data_t *_iter, void *_data);
    // 初始化中断信息
    static bool dtb_init_interrupt_cb(const iter_data_t *_iter, void *_data);

protected:
public:
    // 用于控制处理哪些属性
    static constexpr const uint8_t DT_ITER_BEGIN_NODE = 0x01;
    static constexpr const uint8_t DT_ITER_END_NODE   = 0x02;
    static constexpr const uint8_t DT_ITER_PROP       = 0x04;
    static void                    dtb_init(void);
    static bool                    ttt(const iter_data_t *_iter, void *_data);
    friend std::ostream &          operator<<(std::ostream &     _os,
                                    const iter_data_t &_iter);
};

#endif /* _DTB_H_ */
