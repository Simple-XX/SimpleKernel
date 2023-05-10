
/**
 * @file virtio_mmio_drv.h
 * @brief virtio mmio 驱动头文件
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-12-01<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#ifndef SIMPLEKERNEL_VIRTIO_MMIO_DRV_H
#define SIMPLEKERNEL_VIRTIO_MMIO_DRV_H

#include "common.h"
#include "cstdint"
#include "driver_base.h"
#include "resource.h"
#include "string"
#include "vector"
#include "virtio_dev.h"
#include "virtio_queue.h"

/**
 * @brief virtio,mmio 驱动
 */
class virtio_mmio_drv_t : public driver_base_t {
private:
    /// 魔数
    /// virtio-v1.1#4.2.2
    static constexpr const uint32_t MAGIC_VALUE = 0x74726976;
    /// 版本
    /// virtio-v1.1#4.2.2
    static constexpr const uint32_t VERSION     = 0x02;

    /**
     * @brief virtio mmio 控制寄存器
     * @see virtio-v1.1#4.2.2
     */
    struct virtio_regs_t {
        /// a Little Endian equivalent of the “virt” string: 0x74726976
        uint32_t magic;
        /// Device version number
        // 0x2, Legacy devices(see 4.2.4 Legacy interface) used 0x1.
        uint32_t version;
        /// Virtio Subsystem Device ID
        uint32_t device_id;
        /// Virtio Subsystem Vendor ID
        uint32_t Vendor_id;
        /// Flags representing features the device supports
        uint32_t device_features;
        /// Device (host) features word selection.
        uint32_t device_features_sel;
        uint32_t _reserved0[2];
        /// Flags representing device features understood and activated by
        /// thedriver
        uint32_t driver_features;
        /// Activated (guest) features word selection
        uint32_t driver_features_sel;
        uint32_t _reserved1[2];
        /// Virtual queue index
        uint32_t queue_sel;
        /// Maximum virtual queue size
        uint32_t queue_num_max;
        /// Virtual queue size
        uint32_t queue_num;
        uint32_t _reserved2[2];
        /// Virtual queue ready bit
        uint32_t queue_ready;
        uint32_t _reserved3[2];
        /// Queue notifier
        uint32_t queue_notify;
        uint32_t _reserved4[3];
        /// Interrupt status
        uint32_t interrupt_status;
        /// Interrupt acknowledge
        uint32_t interrupt_ack;
        uint32_t _reserved5[2];
        /// Device status
        uint32_t status;
        uint32_t _reserved6[3];
        /// Virtual queue’s Descriptor Area 64 bit long physical address
        uint32_t queue_desc_low;
        uint32_t queue_desc_high;
        uint32_t _reserved7[2];
        /// Virtual queue’s Driver Area 64 bit long physical address
        uint32_t queue_driver_low;
        uint32_t queue_driver_high;
        uint32_t _reserved8[2];
        /// Virtual queue’s Device Area 64 bit long physical address
        uint32_t queue_device_low;
        uint32_t queue_device_high;
        uint32_t _reserved9[21];
        /// Configuration atomicity value
        uint32_t config_generation;
        /// Configuration space
        uint32_t config[0];
    } __attribute__((packed));

    /**
     * @brief 块设备 feature bits
     * @see virtio-v1.1#5.2.3
     */
    /// Device supports request barriers.
    static constexpr const uint32_t BLK_F_BARRIER               = 0;
    /// Maximum size of any single segment is in size_max.
    static constexpr const uint32_t BLK_F_SIZE_MAX              = 1;
    /// Maximum number of segments in a request is in seg_max.
    static constexpr const uint32_t BLK_F_SEG_MAX               = 2;
    /// Disk-style geometry specified in geometry.
    static constexpr const uint32_t BLK_F_GEOMETRY              = 4;
    /// Device is read-only.
    static constexpr const uint32_t BLK_F_RO                    = 5;
    /// Block size of disk is in blk_size.
    static constexpr const uint32_t BLK_F_BLK_SIZE              = 6;
    /// Cache flush command support.
    static constexpr const uint32_t BLK_F_FLUSH                 = 9;
    /// Device exports information on optimal I/O alignment.
    static constexpr const uint32_t BLK_F_TOPOLOGY              = 10;
    /// Device can toggle its cache between writeback and writethrough modes.
    static constexpr const uint32_t BLK_F_CONFIG_WCE            = 11;
    /// Device can support discard command, maximum discard sectors size in
    /// max_discard_sectors and maximum discard segment number in
    /// max_discard_seg.
    static constexpr const uint32_t BLK_F_DISCARD               = 13;
    /// Devicecansupportwritezeroescommand,maximumwritezeroes sectors size in
    /// max_write_zeroes_sectors and maximum write zeroes segment number in
    /// max_write_- zeroes_seg.
    static constexpr const uint32_t BLK_F_WRITE_ZEROES          = 14;

    /**
     * @brief Reserved Feature Bits
     * @see virtio-v1.1#6
     */
    static constexpr const uint32_t VIRTIO_F_RING_INDIRECT_DESC = 0x1C;
    static constexpr const uint32_t VIRTIO_F_RING_EVENT_IDX     = 0x1D;
    static constexpr const uint32_t VIRTIO_F_VERSION_1          = 0x20;

    /**
     * @brief feature 信息
     */
    struct feature_t {
        /// feature 名称
        mystl::string name;
        /// 第几位
        uint32_t      bit;
        /// 状态
        bool          status;
    };

    /**
     * @brief 公共基础特性
     */
    const feature_t base_features[3] = {
        {"VIRTIO_F_RING_INDIRECT_DESC", VIRTIO_F_RING_INDIRECT_DESC, false},
        {    "VIRTIO_F_RING_EVENT_IDX",     VIRTIO_F_RING_EVENT_IDX, false},
        {         "VIRTIO_F_VERSION_1",          VIRTIO_F_VERSION_1, false},
    };

    /**
     * @brief 块设备配置结构
     * @see virtio-v1.1#5.2.4
     */
    struct virtio_blk_config_t {
        uint64_t capacity;
        uint32_t size_max;
        uint32_t seg_max;

        struct {
            uint16_t cylinders;
            uint8_t  heads;
            uint8_t  sectors;
        } geometry;

        uint32_t blk_size;

        struct {
            // # of logical blocks per physical block (log2)
            uint8_t  physical_block_exp;
            // offset of first aligned logical block
            uint8_t  alignment_offset;
            // suggested minimum  I/O size in blocks
            uint16_t min_io_size;
            // optimal (suggested maximum) I/O size in blocks
            uint32_t opt_io_size;
        } topology;

        uint8_t  writeback;
        uint8_t  unused0[3];
        uint32_t max_discard_sectors;
        uint32_t max_discard_seg;
        uint32_t discard_sector_alignment;
        uint32_t max_write_zeroes_sectors;
        uint32_t max_write_zeroes_seg;
        uint8_t  write_zeroes_may_unmap;
        uint8_t  unused1[3];
    } __attribute__((packed));

    /// 块设备配置信息指针
    virtio_blk_config_t* config;

    feature_t            blk_features[8] = {
        {  NAME2STR(BLK_F_SIZE_MAX),   BLK_F_SIZE_MAX, false},
        {   NAME2STR(BLK_F_SEG_MAX),    BLK_F_SEG_MAX, false},
        {  NAME2STR(BLK_F_GEOMETRY),   BLK_F_GEOMETRY, false},
        {        NAME2STR(BLK_F_RO),         BLK_F_RO, false},
        {  NAME2STR(BLK_F_BLK_SIZE),   BLK_F_BLK_SIZE, false},
        {     NAME2STR(BLK_F_FLUSH),      BLK_F_FLUSH, false},
        {  NAME2STR(BLK_F_TOPOLOGY),   BLK_F_TOPOLOGY, false},
        {NAME2STR(BLK_F_CONFIG_WCE), BLK_F_CONFIG_WCE, false},
    };

    /**
     * @brief 设置设备 features
     * @param  _features        要设置的 feature 向量
     */
    void set_features(const mystl::vector<feature_t>& _features);

    /**
     * @brief 将队列设置传递到相应寄存器
     * @param  _queue_sel       第几个队列，从 0 开始
     */
    void add_to_device(uint32_t _queue_sel);

protected:

public:
    static constexpr const char*   NAME = "virtio,mmio";

    /// virtio mmio 寄存器基地址
    virtio_regs_t*                 regs;
    /// virtio queue，有些设备使用多个队列
    mystl::vector<virtio_queue_t*> queues;

    virtio_mmio_drv_t(const resource_t& _resource);
    ~virtio_mmio_drv_t(void);

    /**
     * @brief 驱动程序的初始化
     * @return true             成功
     * @return false            失败
     */
    bool init(void) override final;

    /**
     * @brief 块设备请求结构
     * @see virtio-v1.1#5.2.6
     */
    struct virtio_blk_req_t {
        /// 请求类型
        /// 读取
        static constexpr const uint32_t IN           = 0;
        /// 写入
        static constexpr const uint32_t OUT          = 1;
        /// 刷新
        static constexpr const uint32_t FLUSH        = 4;
        static constexpr const uint32_t DISCARD      = 11;
        static constexpr const uint32_t WRITE_ZEROES = 13;
        uint32_t                        type;
        uint32_t                        reserved;
        uint64_t                        sector;
        uint8_t*                        data;
        /// 设备返回状态 成功
        static constexpr const uint32_t OK     = 0;
        /// 设备返回状态 设备或驱动出错
        static constexpr const uint32_t IOERR  = 1;
        /// 设备返回状态 不支持的请求
        static constexpr const uint32_t UNSUPP = 2;
        uint8_t                         status;
    } __attribute__((packed));

    static constexpr const size_t VIRTIO_BLK_REQ_HEADER_SIZE = 16;
    static constexpr const size_t VIRTIO_BLK_SECTOR_SIZE     = 512;
    static constexpr const size_t VIRTIO_BLK_REQ_FOOTER_SIZE = 1;

    // 设备基本操作
    // 从设备读
    int                           read(void* _where, void* _buf) override final;
    // 向设备写
    int    write(void* _where, void* _buf) override final;
    // ioctl 控制
    int    ioctl(uint8_t _cmd, void* _buf) override final;
    // 获取设备状态
    int    status(uint8_t _cmd) override final;

    size_t rw(virtio_blk_req_t& _req, void* _buf);
    void   set_intr_ack(void);
    size_t get_queue_len(void);
};

declare_call_back(virtio_mmio_drv_t);

#endif /* SIMPLEKERNEL_VIRTIO_MMIO_DRV_H */
