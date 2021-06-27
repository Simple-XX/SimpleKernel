
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_scsi.h for Simple-XX/SimpleKernel.

#ifndef _VIRTIO_SCSI_H_
#define _VIRTIO_SCSI_H_

#include "stdint.h"
#include "stdlib.h"
#include "virtio.h"

// virtio scsi 设备抽象
class VIRTIO_SCSI : VIRTIO {
private:
protected:
public:
    VIRTIO_SCSI(void *_addr);
    ~VIRTIO_SCSI(void);
    // 设备的读写
    // TODO: 读写缓冲区
    size_t read(void);
    size_t write(void);
};

#endif /* _VIRTIO_SCSI_H_ */
