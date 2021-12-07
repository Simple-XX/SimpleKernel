
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
#include "resource.h"
#include "drv.h"

// virtio 设备类型
class virtio_mmio_drv_t : public drv_t {
private:
    // virtio 设备类型
    // virtio-v1.1#5
    typedef enum : uint8_t {
        RESERVED = 0x00,
        NETWORK_CARD,
        BLOCK_DEVICE,
        CONSOLE,
        ENTROPY_SOURCE,
        MEMORY_BALLOONING,
        IOMEMORY,
        RPMSG,
        SCSI_HOST,
        NINEP_TRANSPORT,
        MAC_80211_WLAN,
        RPROC_SERIAL,
        VIRTIO_CAIF,
        MEMORY_BALLOON,
        GPU_DEVICE = 0x10,
        TIMER_CLOCK_DEVICE,
        INPUT_DEVICE,
        SOCKET_DEVICE,
        CRYPTO_DEVICE,
        SIGNAL_DISTRIBUTION_MODULE,
        PSTORE_DEVICE,
        IOMMU_DEVICE,
        MEMORY_DEVICE,
    } virt_device_type_t;

    // virtio 设备类型名名称
    const char *const virtio_device_name[25] = {
        "reserved (invalid)",
        "network card",
        "block device",
        "console",
        "entropy source",
        "memory ballooning(traditional)",
        "ioMemory",
        "rpmsg",
        "SCSI host",
        "9P transport",
        "mac80211 wlan",
        "rproc serial",
        "virtio CAIF",
        "memory balloon",
        "null",
        "null",
        "GPU device",
        "Timer / Clock device",
        "Input device",
        "Socket device",
        "Crypto device",
        "Signal Distribution Module",
        "pstore device",
        "IOMMU device",
        "Memory device",
    };

    static constexpr const uint64_t MAGIC_VALUE = 0x74726976;
    static constexpr const uint64_t VERSION     = 0x02;

    // Device Status Field
    // virtio-v1.1#2.1
    static constexpr const uint64_t DEVICE_STATUS_ACKNOWLEDGE        = 0x1;
    static constexpr const uint64_t DEVICE_STATUS_DRIVER             = 0x2;
    static constexpr const uint64_t DEVICE_STATUS_DRIVER_OK          = 0x4;
    static constexpr const uint64_t DEVICE_STATUS_FEATURES_OK        = 0x8;
    static constexpr const uint64_t DEVICE_STATUS_DEVICE_NEEDS_RESET = 0x40;
    static constexpr const uint64_t DEVICE_STATUS_FAILED             = 0x80;

    // Device Status Field
    // virtio-v1.1#6
    static constexpr const uint64_t VIRTIO_F_RING_INDIRECT_DESC = 0x1C;
    static constexpr const uint64_t VIRTIO_F_RING_EVENT_IDX     = 0x1D;
    static constexpr const uint64_t VIRTIO_F_VERSION_1          = 0x20;

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
        {NAME2STR(VIRTIO_F_RING_INDIRECT_DESC), VIRTIO_F_RING_INDIRECT_DESC,
         false},
        {NAME2STR(VIRTIO_F_RING_EVENT_IDX), VIRTIO_F_RING_EVENT_IDX, false},
        {NAME2STR(VIRTIO_F_VERSION_1), VIRTIO_F_VERSION_1, false},
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
    struct virtio_blk_config {
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

    virtio_blk_config *config;

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
};

declare_call_back(virtio_mmio_drv_t);

#endif /* _VIRTIO_MMIO_DRV_H_ */
