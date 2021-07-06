
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_bus.h for Simple-XX/SimpleKernel.

#ifndef _VIRTIO_BUS_H_
#define _VIRTIO_BUS_H_

#include "stdint.h"
#include "vector"
#include "string"
#include "dev.h"
#include "stdio.h"

class virtio_bus_t : bus_t {
private:
protected:
public:
    virtio_bus_t(mystl::vector<resource_t *> _res);
    ~virtio_bus_t(void);
};

#endif /* _VIRTIO_BUS_H_ */
