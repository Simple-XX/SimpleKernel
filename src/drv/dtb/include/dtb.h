
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// dtb.h for Simple-XX/SimpleKernel.

#ifndef _DTB_H_
#define _DTB_H_

#include "stdint.h"
#include "string"
#include "vector"
#include "unordered_map"
#include "resource.h"

// https://e-mailky.github.io/2016-12-06-dts-introduce
// https://e-mailky.github.io/2019-01-14-dts-1
// https://e-mailky.github.io/2019-01-14-dts-2
// https://e-mailky.github.io/2019-01-14-dts-3

// TODO: 删除多余代码
// TODO: 完善功能
// TODO: 优化
struct node_begin_t {
    // begin 标识
    uint32_t tag;
    // node 名称，4 字节对齐的字符串
    uint8_t name[0];
} __attribute__((packed));

struct fdt_property_t {
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

// 标准属性
// devicetree-specification-v0.3#2.3.1
struct standard_props_t {
    // 属性列表
    static constexpr const char *const COMPATIBLE  = "compatible";
    static constexpr const char *const MODEL       = "model";
    static constexpr const char *const PHANDLE     = "phandle";
    static constexpr const char *const STATUS      = "status";
    static constexpr const char *const ADDR_CELLS  = "#address-cells";
    static constexpr const char *const SIZE_CELLS  = "#size-cells";
    static constexpr const char *const REG         = "reg";
    static constexpr const char *const VIRTUAL_REG = "virtual-reg";
    static constexpr const char *const RANGES      = "ranges";
    static constexpr const char *const DMA_RANGES  = "dma-ranges";
    // 各个属性的值
    // TODO: 这里应该是 string 向量
    mystl::string           compatible;
    mystl::string           model;
    uint32_t                phandle;
    mystl::string           status;
    uint32_t                addr_cells;
    uint32_t                size_cells;
    mystl::vector<uint32_t> reg;
    uint32_t                virt_reg;
    mystl::vector<uint32_t> ranges;
    mystl::vector<uint32_t> dma_ranges;
};

// 中断属性
// devicetree-specification-v0.3#2.4.1
struct interrupt_device_props_t {
    // 属性列表
    static constexpr const char *const INTERRUPTS       = "interrupts";
    static constexpr const char *const INTERRUPT_PARENT = "interrupt-parent";
    static constexpr const char *const INTERRUPTS_EXTENDED =
        "interrupts-extended";
    // 各个属性的值
    // TODO: 这里应该是 uint32_t 向量
    uint32_t                interrupts;
    mystl::vector<uint32_t> interrupt_parent;
    mystl::vector<uint32_t> interrupts_extended;
};

// 中断控制器属性
// devicetree-specification-v0.3#2.4.2
struct interrupt_controllers_props_t {
    // 属性列表
    static constexpr const char *const INTERRUPT_CELLS = "#interrupt-cells";
    static constexpr const char *const INTERRUPT_CONTROLLER =
        "interrupt-controller";
    // 各个属性的值
    uint32_t interrupt_cells;
    bool     interrupt_controller;
    interrupt_controllers_props_t(void);
    ~interrupt_controllers_props_t(void);
};

// 中断控制器属性
// devicetree-specification-v0.3#2.4.3
struct interrupt_nexus_props_t {
    // 属性列表
    static constexpr const char *const INTERRUPT_MAP = "interrupt-map";
    static constexpr const char *const INTERRUPT_MAP_MASK =
        "interrupt-map-mask";
    static constexpr const char *const INTERRUPT_CELLS = "#interrupt-cells";

    // 各个属性的值
    mystl::vector<uint32_t> interrupt_map;
    mystl::vector<uint32_t> interrupt_map_mask;
    uint32_t                interrupt_cells;
    interrupt_nexus_props_t(void);
    ~interrupt_nexus_props_t(void);
};

class dtb_prop_node_t {
private:
protected:
public:
    // 节点名
    mystl::string name;
    // 节点的属性列表
    // 标准属性
    standard_props_t standard;
    // 中断设备属性
    interrupt_device_props_t interrupt_device;
    // 子节点指针
    mystl::vector<dtb_prop_node_t *> children;
    dtb_prop_node_t(void) {
        return;
    }
    dtb_prop_node_t(const mystl::string &_name);
    ~dtb_prop_node_t(void);
    // 添加属性
    // _name: 属性名
    // _addr: 数据地址
    // _len: 数据长度，单位为 4bytes
    void add_prop(mystl::string _name, uint32_t *_addr, uint32_t _len);
    // 添加子节点
    void add_child(dtb_prop_node_t *_child);
};

// See devicetree-specification-v0.3.pdf for more info
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
        uint64_t address;
        // 长度
        uint64_t size;
        fdt_reserve_entry_t(const fdt_reserve_entry_t *_addr);
        ~fdt_reserve_entry_t(void);
    };

    // 以下几个数据是不变的
    // 节点信息
    const fdt_header_t header;
    // 大小
    const uint32_t size;
    // 数据区地址
    const uint32_t *data_addr;
    // 数据区长度
    const uint32_t data_size;
    // 字符串区地址
    const uint8_t *string_addr;
    // 字符串区长度
    const uint32_t string_size;
    // 保留区地址
    const fdt_reserve_entry_t *reserve_addr;
    // 保留区信息向量
    mystl::vector<fdt_reserve_entry_t> reserve;
    //节点指针
    mystl::vector<dtb_prop_node_t *> nodes;
    // 获取名称
    char *get_string(uint64_t _off);
    // reserve 区域初始化
    void reserve_init(void);
    // 初始化 dtb 中的各个节点
    void nodes_init(void);

protected:
public:
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

    DTB(void);
    ~DTB(void);
    // 根据设备名查询设备信息
    // TODO: 应该返回一个设备使用资源的 list，因为一个设备可能使用多个资源
    const mystl::vector<resource_t *> find(mystl::string _name);
    // 迭代器
    static void dtb_iter(dtb_iter_fun_t _fun, void *_data);
    // 输出内存信息
    static bool printf_memory(dtb_iter_data_t *_iter_data, void *);
    // 获取内存信息
    static bool get_memory(dtb_iter_data_t *_iter_data, void *_data);
    // 获取 CLINT
    static bool get_clint(dtb_iter_data_t *_iter, void *_data);
    // 获取 PLIC
    static bool get_plic(dtb_iter_data_t *_iter, void *_data);
};

extern "C" size_t dtb_init_hart;

#endif /* _DTB_H_ */
