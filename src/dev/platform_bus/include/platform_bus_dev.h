
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// platform_bus_dev.h for Simple-XX/SimpleKernel.

#ifndef _PLATFORM_BUS_DEV_H_
#define _PLATFORM_BUS_DEV_H_

#include "vector"
#include "string"
#include "dev.h"
#include "bus.h"
#include "resource.h"
#include "virtio_dev.h"

// virtio bus 设备
// 这是一个虚拟总线
class platform_bus_dev_t : public bus_t {
private:
protected:
public:
    platform_bus_dev_t(void);
    platform_bus_dev_t(const resource_t &_resource);
    ~platform_bus_dev_t(void);
};

#endif /* _PLATFORM_BUS_DEV_H_ */
