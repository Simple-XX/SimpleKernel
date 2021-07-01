
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_blk.h for Simple-XX/SimpleKernel.

#ifndef _VIRTIO_BLK_H_
#define _VIRTIO_BLK_H_

#include "stdint.h"
#include "stdlib.h"
#include "virtio.h"
#include "virtio_queue.h"

// virtio scsi 设备抽象
// virtio-v1.1#5.2
class VIRTIO_BLK : VIRTIO {
private:
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

protected:
public:
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
    uint8_t status;
    VIRTIO_BLK(void *_addr);
    ~VIRTIO_BLK(void);
    // 设备的读写
    // TODO: 读写缓冲区
    size_t rw(virtio_blk_req_t &_req, void *_buf);
};

#endif /* _VIRTIO_BLK_H_ */
