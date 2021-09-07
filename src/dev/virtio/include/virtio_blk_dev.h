
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_blk_dev.h for Simple-XX/SimpleKernel.

#ifndef _VIRTIO_BLK_DEV_H_
#define _VIRTIO_BLK_DEV_H_

#include "virtio_dev.h"

// virtio blk 设备
class virtio_blk_dev_t : virtio_dev_t {
private:
protected:
public:
    virtio_blk_dev_t(const resource_t &_resource);
    ~virtio_blk_dev_t(void);
};

#endif /* _VIRTIO_BLK_DEV_H_ */
