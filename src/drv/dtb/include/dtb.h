
/**
 * @file dtb.h
 * @brief dtb 解析
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * Based on https://github.com/brenns10/sos
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-09-18<td>digmouse233<td>迁移到 doxygen
 * </table>
 */

#ifndef _DTB_H_
#define _DTB_H_

#include "stdint.h"
#include "stdbool.h"
#include "boot_info.h"
#include "resource.h"
#include "endian.h"
#include "iostream"

/// @see devicetree-specification-v0.3.pdf
/// @see https://e-mailky.github.io/2016-12-06-dts-introduce
/// @see https://e-mailky.github.io/2019-01-14-dts-1
/// @see https://e-mailky.github.io/2019-01-14-dts-2
/// @see https://e-mailky.github.io/2019-01-14-dts-3

/**
 * @brief dtb 接口抽象
 */
class DTB {
private:
    /// @see devicetree-specification-v0.3.pdf#5.4
    /// node 开始标记
    static constexpr const uint32_t FDT_BEGIN_NODE = 0x1;
    /// node 结束标记
    static constexpr const uint32_t FDT_END_NODE = 0x2;
    /// 属性开始标记
    static constexpr const uint32_t FDT_PROP = 0x3;
    /// 无效标记
    static constexpr const uint32_t FDT_NOP = 0x4;
    /// 数据区结束标记
    static constexpr const uint32_t FDT_END = 0x9;

    /// @see devicetree-specification-v0.3.pdf#5.1
    /// 魔数
    static constexpr const uint32_t FDT_MAGIC = 0xD00DFEED;
    /// 版本 17
    static constexpr const uint32_t FDT_VERSION = 0x11;

    /**
     * @brief fdt 格式头
     */
    struct fdt_header_t {
        /// 魔数，0xD00DFEED
        uint32_t magic;
        /// 整个 fdt 结构的大小
        uint32_t totalsize;
        /// 数据区相对 header 地址的偏移
        uint32_t off_dt_struct;
        /// 字符串区相对 header 地址的偏移
        uint32_t off_dt_strings;
        /// 内存保留相对 header 地址的偏移
        uint32_t off_mem_rsvmap;
        /// fdt 版本，17
        uint32_t version;
        /// 最新的兼容版本
        uint32_t last_comp_version;
        /// 启动 cpu 的 id
        uint32_t boot_cpuid_phys;
        /// 字符串区长度
        uint32_t size_dt_strings;
        /// 数据区长度
        uint32_t size_dt_struct;
    };

    /**
     * @brief 内存保留区信息
     * @see devicetree-specification-v0.3.pdf#5.3.2
     */
    struct fdt_reserve_entry_t {
        /// 地址
        uint32_t addr_be;
        uint32_t addr_le;
        /// 长度
        uint32_t size_be;
        uint32_t size_le;
    };

    /**
     * @brief 属性节点格式，没有使用，仅用于参考
     */
    struct fdt_property_t {
        /// 第一个字节表示 type
        uint32_t tag;
        /// 表示 property value 的长度，byte 为单位
        uint32_t len;
        /// property 的名称存放在 string block 区域，nameoff 表示其在 string
        /// block 的偏移
        uint32_t nameoff;
        /// property value值，作为额外数据以'\0'结尾的字符串形式存储 structure
        /// block, 32 - bits对齐，不够的位用 0x0 补齐
        uint32_t data[0];
    };

    /// 路径最大深度
    static constexpr const size_t MAX_DEPTH = 16;
    /// 最大节点数
    static constexpr const size_t MAX_NODES = 128;
    /// 最大属性数
    static constexpr const size_t PROP_MAX = 16;

    /**
     * @brief dtb 信息
     */
    struct dtb_info_t {
        /// dtb 头
        fdt_header_t *header;
        /// 保留区
        fdt_reserve_entry_t *reserved;
        /// 数据区
        uintptr_t data;
        /// 字符区
        uintptr_t str;
    };

    /**
     * @brief 属性信息
     */
    struct prop_t {
        /// 属性名
        char *name;
        /// 属性地址
        uintptr_t addr;
        /// 属性长度
        size_t len;
    };

    /**
     * @brief 路径
     */
    struct path_t {
        /// 当前路径
        char *path[MAX_DEPTH];
        /// 长度
        size_t len;
        /**
         * @brief 全部匹配
         * @param  _path           要比较的路径
         * @return true            相同
         * @return false           不同
         */
        bool operator==(const path_t *_path);
        bool operator==(const char *_path);
    };

    /**
     * @brief 节点数据
     */
    struct node_t {
        /// 节点路径
        path_t path;
        /// 节点地址
        uint32_t *addr;
        /// 父节点
        node_t *parent;
        /// 中断父节点
        node_t *interrupt_parent;
        /// 1 cell == 4 bytes
        /// 地址长度 单位为 bytes
        uint32_t address_cells;
        /// 长度长度 单位为 bytes
        uint32_t size_cells;
        /// 中断长度 单位为 bytes
        uint32_t interrupt_cells;
        uint32_t phandle;
        /// 路径深度
        uint8_t depth;
        /// 属性
        prop_t props[PROP_MAX];
        /// 属性数
        size_t prop_count;
        /// 节点数
        static size_t count;
        /// 查找节点中的键值对
        bool find(const char *_prop_name, const char *_val);
    };

    /**
     * @brief 迭代变量
     */
    struct iter_data_t {
        /// 路径，不包括节点名
        path_t path;
        /// 节点地址
        uint32_t *addr;
        /// 节点类型
        uint32_t type;
        /// 如果节点类型为 PROP， 保存节点属性名
        char *prop_name;
        /// 如果节点类型为 PROP， 保存属性长度 单位为 byte
        uint32_t prop_len;
        /// 如果节点类型为 PROP， 保存属性地址
        uint32_t *prop_addr;
        /// 在 nodes 数组的下标
        uint8_t nodes_idx;
    };

    // 部分属性及格式
    /// @see devicetree-specification-v0.3#2.3
    /// @see  devicetree-specification-v0.3#2.4.1
    /**
     * @brief 格式
     */
    enum dt_fmt_t {
        /// 未知
        FMT_UNKNOWN = 0,
        /// 空
        FMT_EMPTY,
        /// uint32_t
        FMT_U32,
        /// uint64_t
        FMT_U64,
        /// 字符串
        FMT_STRING,
        /// phandle
        FMT_PHANDLE,
        /// 字符串列表
        FMT_STRINGLIST,
        /// reg
        FMT_REG,
        /// ranges
        FMT_RANGES,
    };

    /**
     * @brief 用于 get_fmt()
     */
    struct dt_prop_fmt_t {
        /// 属性名
        char *prop_name;
        /// 格式
        enum dt_fmt_t fmt;
    };

    /**
     * @brief 用于 get_fmt()
     * @see 格式信息请查看 devicetree-specification-v0.3#2.3,#2.4 等部分
     */
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

    /**
     * @brief 查找 _prop_name 在 dt_fmt_t 的索引
     * @param  _prop_name      要查找的属性
     * @return dt_fmt_t        在 dt_fmt_t 中的索引
     */
    dt_fmt_t get_fmt(const char *_prop_name);

    /**
     * @brief phandles 与 node 的映射关系
     */
    struct phandle_map_t {
        uint32_t phandle;
        node_t  *node;
        /// phandle 数量
        static size_t count;
    };

    /// dtb 信息
    static dtb_info_t dtb_info;
    /// 节点数组
    static node_t nodes[MAX_NODES];
    /// phandle 数组
    static phandle_map_t phandle_map[MAX_NODES];

    /**
     * @brief 输出 reserved 内存
     */
    void dtb_mem_reserved(void);

    /**
     * @brief 迭代函数
     * @param  _cb_flags       要迭代的标签
     * @param  _cb             迭代操作
     * @param  _data           要传递的数据
     * @param _addr            dtb 数据地址
     */
    void dtb_iter(uint8_t _cb_flags,
                         bool (*_cb)(const iter_data_t *, void *), void *_data,
                         uintptr_t _addr = dtb_info.data);

    /**
     * @brief 查找 phandle 映射
     * @param  _phandle        要查找的 phandle
     * @return node_t*         _phandle 指向的节点
     */
    node_t *get_phandle(uint32_t _phandle);

    /**
     * @brief 初始化节点
     * @param  _iter           迭代变量
     * @param  _data           数据，未使用
     * @return true            成功
     * @return false           失败
     */
    static bool dtb_init_cb(const iter_data_t *_iter, void *_data);

    /**
     * @brief 初始化中断信息
     * @param  _iter           迭代变量
     * @param  _data           数据，未使用
     * @return true            成功
     * @return false           失败
     */
    static bool dtb_init_interrupt_cb(const iter_data_t *_iter, void *_data);

    /**
     * @brief 输出不定长度的数据
     * @param  _iter           要输出的迭代对象
     * @param  _cells          cell 数
     * @param  _len            长度
     * @todo 补充说明
     */
    void print_attr_propenc(const iter_data_t *_iter, size_t *_cells,
                                   size_t _len);

    /**
     * @brief 填充 resource_t
     * @param  _resource       被填充的
     * @param  _node           源节点
     * @param  _prop           填充的数据
     */
    void fill_resource(resource_t *_resource, const node_t *_node,
                              const prop_t *_prop);

    /**
     * @brief 通过路径寻找节点
     * @param  _path            路径
     * @return node_t*          找到的节点
     */
    node_t *find_node_via_path(const char *_path);

    /// 用于标记是否第一次 init
    static bool inited;

protected:
public:
    // 用于控制处理哪些属性
    /// 处理节点开始
    static constexpr const uint8_t DT_ITER_BEGIN_NODE = 0x01;
    /// 处理节点结束
    static constexpr const uint8_t DT_ITER_END_NODE = 0x02;
    /// 处理节点属性
    static constexpr const uint8_t DT_ITER_PROP = 0x04;

    /**
     * @brief 获取单例
     * @return DTB&             静态对象
     */
    static DTB &get_instance(void);

    /**
     * @brief 初始化
     * @return true            成功
     * @return false           失败
     */
    bool dtb_init(void);

    /**
     * @brief 根据路径查找节点，返回使用的资源
     * @param  _path            节点路径
     * @param  _resource        资源
     * @return true             成功
     * @return false            失败
     */
    bool find_via_path(const char *_path, resource_t *_resource);

    /**
     * @brief 根据节点名进行前缀查找
     * @param  _prefix          要查找节点的前缀
     * @param  _resource        结果数组
     * @return size_t           _resource 长度
     * @note 根据节点 @ 前的名称查找，可能返回多个 resource
     */
    size_t find_via_prefix(const char *_prefix, resource_t *_resource);

    /**
     * @brief iter 输出
     * @param  _os             输出流
     * @param  _iter           要输出的 iter
     * @return std::ostream&   输出流
     */
    friend std::ostream &operator<<(std::ostream      &_os,
                                    const iter_data_t &_iter);
    /**
     * @brief 路径输出
     * @param  _os             输出流
     * @param  _path           要输出的 path
     * @return std::ostream&   输出流
     */
    friend std::ostream &operator<<(std::ostream &_os, const path_t &_path);
};

namespace BOOT_INFO {
/// 保存 sbi 传递的启动核
extern "C" size_t dtb_init_hart;
}; // namespace BOOT_INFO

#endif /* _DTB_H_ */
