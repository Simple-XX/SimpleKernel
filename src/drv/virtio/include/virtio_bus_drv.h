
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_bus_drv.h for Simple-XX/SimpleKernel.

#ifndef _VIRTIO_BUS_DRV_H_
#define _VIRTIO_BUS_DRV_H_

#include "stdint.h"
#include "vector"
#include "string"
#include "dev.h"
#include "drv.h"
#include "stdio.h"

struct virtio_bus_drv_t : drv_t {
private:
protected:
public:
    // TODO: 填充内容
    virtio_bus_drv_t(void) {
        name = "virtio_mmio drv";
        return;
    }

    ~virtio_bus_drv_t(void) {
        return;
    }

    bool init(void) {
        printf("virtio_bus_drv_t init.\n");
        return true;
    }

    void read(void) {
        return;
    }

    void write(void) {
        return;
    }
};

#endif /* _VIRTIO_BUS_DRV_H_ */
