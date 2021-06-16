
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_blk.h for Simple-XX/SimpleKernel.

#ifndef _VIRTIO_BLK_H_
#define _VIRTIO_BLK_H_

#include "stdint.h"

// virtio-v1.1#5.2
namespace VIRTIO_BLK {
    // 块设备 features
    // virtio-v1.1#5.2.3
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
    // Devicecantoggleitscachebetweenwritebackandwritethroughmodes.
    static constexpr const uint64_t BLK_F_CONFIG_WCE = 11;
    // Device can support discard command, maximum discard sectors size in
    // max_discard_sectors and maximum discard segment number in
    // max_discard_seg.
    static constexpr const uint64_t BLK_F_DISCARD = 13;
    // Devicecansupportwritezeroescommand,maximumwritezeroes sectors size in
    // max_write_zeroes_sectors and maximum write zeroes segment number in
    // max_write_- zeroes_seg.
    static constexpr const uint64_t BLK_F_WRITE_ZEROES = 14;

    static constexpr const uint64_t BLK_S_OK     = 0;
    static constexpr const uint64_t BLK_S_IOERR  = 1;
    static constexpr const uint64_t BLK_S_UNSUPP = 2;

    // virtio-v1.1#5.2.6
    struct virtio_blk_req {
        // virtio_blk_req: type
        static constexpr const uint64_t IN           = 0;
        static constexpr const uint64_t OUT          = 1;
        static constexpr const uint64_t FLUSH        = 4;
        static constexpr const uint64_t DISCARD      = 11;
        static constexpr const uint64_t WRITE_ZEROES = 13;
        uint32_t                        type;
        uint32_t                        reserved;
        uint64_t                        sector;
        uint8_t *                       data;
        uint8_t                         status;
    };
};

#endif /* _VIRTIO_BLK_H_ */
