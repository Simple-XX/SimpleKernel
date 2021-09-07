
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_dev.h for Simple-XX/SimpleKernel.

#ifndef _VIRTIO_DEV_H_
#define _VIRTIO_DEV_H_

#include "dev.h"

// virtio 设备类型
class virtio_dev_t : public dev_t {
private:
    // 寄存器地址
    // TODO
    void *addr_start;
    void *addr_end;
    // 中断号
    uint8_t irq_no;

protected:
public:
    virtio_dev_t(const resource_t &_resource);
    virtual ~virtio_dev_t(void) = 0;
};

#endif /* _VIRTIO_DEV_H_ */
