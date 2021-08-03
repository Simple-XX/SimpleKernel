
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_bus_dev.h for Simple-XX/SimpleKernel.

#ifndef _VIRTIO_BUS_DEV_H_
#define _VIRTIO_BUS_DEV_H_

#include "vector"
#include "string"
#include "dev.h"
#include "bus.h"
#include "resource.h"
#include "virtio_dev.h"

// virtio bus 设备
class virtio_bus_dev_t : bus_t {
private:
    // 总线上的设备数
    uint32_t dev_num;
    // 设备向量
    mystl::vector<virtio_dev_t *> devs;

protected:
public:
    virtio_bus_dev_t(const mystl::vector<resource_t> &_res);
    ~virtio_bus_dev_t(void);
};

#endif /* _VIRTIO_BUS_DEV_H_ */
