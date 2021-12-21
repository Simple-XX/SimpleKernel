
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
 * <tr><td>2021-12-01<td>MRNIU<td>迁移到 doxygen
 * </table>
 */

#ifndef _VIRTIO_MMIO_DRV_H_
#define _VIRTIO_MMIO_DRV_H_

#include "stdint.h"
#include "common.h"
#include "vector"
#include "string"
#include "virtio_queue.h"
#include "virtio_dev.h"
#include "resource.h"
#include "drv.h"

// virtio 设备类型
class virtio_mmio_drv_t : public drv_t {
private:
    static constexpr const uint64_t MAGIC_VALUE = 0x74726976;
    static constexpr const uint64_t VERSION     = 0x02;

    // virtio mmio 控制寄存器
    // virtio-v1.1#4.2.2
    struct virtio_regs_t {
        // a Little Endian equivalent of the “virt” string: 0x74726976
        uint32_t magic;
        // Device version number
        // 0x2, Legacy devices(see 4.2.4 Legacy interface) used 0x1.
        uint32_t version;
        // Virtio Subsystem Device ID
        uint32_t device_id;
        uint32_t Vendor_id;
        uint32_t device_features;
        uint32_t device_features_sel;
        uint32_t _reserved0[2];
        uint32_t driver_features;
        uint32_t driver_features_sel;
        uint32_t _reserved1[2];
        uint32_t queue_sel;
        uint32_t queue_num_max;
        uint32_t queue_num;
        uint32_t _reserved2[2];
        uint32_t queue_ready;
        uint32_t _reserved3[2];
        uint32_t queue_notify;
        uint32_t _reserved4[3];
        uint32_t interrupt_status;
        uint32_t interrupt_ack;
        uint32_t _reserved5[2];
        uint32_t status;
        uint32_t _reserved6[3];
        uint32_t queue_desc_low;
        uint32_t queue_desc_high;
        uint32_t _reserved7[2];
        uint32_t queue_driver_low;
        uint32_t queue_driver_high;
        uint32_t _reserved8[2];
        uint32_t queue_device_low;
        uint32_t queue_device_high;
        uint32_t _reserved9[21];
        uint32_t config_generation;
        uint32_t config[0];
    } __attribute__((packed));

    // feature 信息
    struct feature_t {
        // feature 名称
        mystl::string name;
        // 第几位
        uint32_t bit;
        // 状态
        bool status;
    };

    feature_t base_features[3] = {
        {NAME2STR(VIRTIO_F_RING_INDIRECT_DESC),
         virtio_dev_t::VIRTIO_F_RING_INDIRECT_DESC, false},
        {NAME2STR(VIRTIO_F_RING_EVENT_IDX),
         virtio_dev_t::VIRTIO_F_RING_EVENT_IDX, false},
        {NAME2STR(VIRTIO_F_VERSION_1), virtio_dev_t::VIRTIO_F_VERSION_1, false},
    };

    // 块设备 feature bits
    // virtio-v1.1#5.2.3
    // Device supports request barriers.
    static constexpr const uint64_t BLK_F_BARRIER = 0;
    // Maximum size of any single segment is in size_max.
    static constexpr const uint64_t BLK_F_SIZE_MAX = 1;
    // Maximum number of segments in a request is in seg_max.
    static constexpr const uint64_t BLK_F_SEG_MAX = 2;
    // Disk-style geometry specified in geometry.
    static constexpr const uint64_t BLK_F_GEOMETRY = 4;
    // Device is read-only.
    static constexpr const uint64_t BLK_F_RO = 5;
    // Block size of disk is in blk_size.
    static constexpr const uint64_t BLK_F_BLK_SIZE = 6;
    // Cache flush command support.
    static constexpr const uint64_t BLK_F_FLUSH = 9;
    // Device exports information on optimal I/O alignment.
    static constexpr const uint64_t BLK_F_TOPOLOGY = 10;
    // Device can toggle its cache between writeback and writethrough modes.
    static constexpr const uint64_t BLK_F_CONFIG_WCE = 11;
    // Device can support discard command, maximum discard sectors size in
    // max_discard_sectors and maximum discard segment number in
    // max_discard_seg.
    static constexpr const uint64_t BLK_F_DISCARD = 13;
    // Devicecansupportwritezeroescommand,maximumwritezeroes sectors size in
    // max_write_zeroes_sectors and maximum write zeroes segment number in
    // max_write_- zeroes_seg.
    static constexpr const uint64_t BLK_F_WRITE_ZEROES = 14;

    // virtio-v1.1#5.2.4
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
            uint8_t physical_block_exp;
            // offset of first aligned logical block
            uint8_t alignment_offset;
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

    virtio_blk_config_t *config;

    feature_t blk_features[8] = {
        {NAME2STR(BLK_F_SIZE_MAX), BLK_F_SIZE_MAX, false},
        {NAME2STR(BLK_F_SEG_MAX), BLK_F_SEG_MAX, false},
        {NAME2STR(BLK_F_GEOMETRY), BLK_F_GEOMETRY, false},
        {NAME2STR(BLK_F_RO), BLK_F_RO, false},
        {NAME2STR(BLK_F_BLK_SIZE), BLK_F_BLK_SIZE, false},
        {NAME2STR(BLK_F_FLUSH), BLK_F_FLUSH, false},
        {NAME2STR(BLK_F_TOPOLOGY), BLK_F_TOPOLOGY, false},
        {NAME2STR(BLK_F_CONFIG_WCE), BLK_F_CONFIG_WCE, false},
    };

    // virtio mmio 寄存器基地址
    virtio_regs_t *regs;
    // virtio queue，有些设备使用多个队列
    mystl::vector<virtio_queue_t *> queues;
    // 设置 features
    bool set_features(const mystl::vector<feature_t> &_features);
    // 将队列设置传递到相应寄存器
    // _queue_sel: 第几个队列，从 0 开始
    void add_to_device(uint32_t _queue_sel);

protected:
public:
    virtio_mmio_drv_t(void);
    virtio_mmio_drv_t(const void *_addr);
    virtio_mmio_drv_t(const resource_t &_resource);
    ~virtio_mmio_drv_t(void);
    // 驱动操作
    bool init(void) override final;
    // virtio-v1.1#5.2.6
    struct virtio_blk_req_t {
        // virtio_blk_req: type
        static constexpr const uint64_t IN           = 0;
        static constexpr const uint64_t OUT          = 1;
        static constexpr const uint64_t FLUSH        = 4;
        static constexpr const uint64_t DISCARD      = 11;
        static constexpr const uint64_t WRITE_ZEROES = 13;
        uint32_t                        type;
        uint32_t                        reserved;
        uint64_t                        sector;
        // uint8_t *                       data;
        // status type
        static constexpr const uint64_t OK     = 0;
        static constexpr const uint64_t IOERR  = 1;
        static constexpr const uint64_t UNSUPP = 2;
        // uint8_t                         status;
    } __attribute__((packed));
    size_t rw(virtio_blk_req_t &_req, void *_buf);
};

declare_call_back(virtio_mmio_drv_t);

#endif /* _VIRTIO_MMIO_DRV_H_ */
