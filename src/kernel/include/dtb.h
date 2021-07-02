
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

// https://e-mailky.github.io/2016-12-06-dts-introduce
// https://e-mailky.github.io/2019-01-14-dts-1
// https://e-mailky.github.io/2019-01-14-dts-2
// https://e-mailky.github.io/2019-01-14-dts-3

// TODO: 删除多余代码
// TODO: 完善功能
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
    mystl::vector<uint32_t> interrupts;
    mystl::vector<uint32_t> interrupt_parent;
    mystl::vector<uint32_t> interrupts_extended;
    interrupt_device_props_t(void);
    ~interrupt_device_props_t(void);
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
    standard_props_t standard;
    // 子节点指针
    mystl::vector<dtb_prop_node_t *> children;
    dtb_prop_node_t(mystl::string &_name);
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

    // devicetree-specification-v0.3.pdf#5.1
    static constexpr const uint32_t FDT_MAGIC = 0xD00DFEED;
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
        uint64_t address;
        // 长度
        uint64_t size;
    };

    // dtb 结构地址
    void *addr;
    // 大小
    uint32_t size;
    // 数据区地址
    uint64_t data_addr;
    // 数据区长度
    uint32_t data_size;
    // 字符串区地址
    uint64_t string_addr;
    // 字符串区长度
    uint32_t string_size;
    // 保留区地址
    uint64_t reserve_addr;
    // 保留区向量
    mystl::vector<fdt_reserve_entry_t> reserve;
    //节点指针
    mystl::vector<dtb_prop_node_t *> nodes;
    // 获取名称
    char *get_string(uint64_t _off);
    // 递归遍历所有 node
    // 返回新的位置和子节点
    mystl::pair<dtb_prop_node_t *, uint8_t *> get_node(uint8_t *_pos);

    void tmp(void);

protected:
public:
    DTB(void);
    ~DTB(void);
    // 根据设备名查询设备信息
    // 返回一个 list，因为同类设备可能有多个
    const mystl::vector<dtb_prop_node_t *> find(mystl::string _name);
};

// 在启动阶段保存 dtb 地址
// TODO: 仅传递 dtb 地址参数
extern "C" void dtb_preinit(uint32_t, void *_addr);

// dtb
// 首先获取到 dtb 地址
// 每个节点的默认属性

#endif /* _DTB_H_ */
