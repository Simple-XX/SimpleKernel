
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_drv.h for Simple-XX/SimpleKernel.

#ifndef _VIRTIO_DRV_H_
#define _VIRTIO_DRV_H_

#include "stdint.h"
#include "common.h"
#include "vector"
#include "string"
#include "virtio_queue.h"
#include "dtb.h"
#include "drv.h"
#include "virtio_dev.h"

// See
// http://docs.oasis-open.org/virtio/virtio/v1.0/csprd01/virtio-v1.0-csprd01.html#x1-530002
// for more info

// virtio 设备驱动
class virtio_drv_t : public drv_t {
private:
protected:
public:
    virtio_drv_t(void);
    virtual ~virtio_drv_t(void);
};

#endif /* _VIRTIO_DRV_H_ */
