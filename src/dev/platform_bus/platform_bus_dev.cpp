
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// platform_bus_dev.cpp for Simple-XX/SimpleKernel.

#include "platform_bus_dev.h"
#include "platform_bus_drv.h"

platform_bus_dev_t::platform_bus_dev_t(void) {
    drv_name = "platform_bus driver";
    return;
}

platform_bus_dev_t::platform_bus_dev_t(const resource_t &_resource)
    : bus_dev_t(_resource) {
    // platform 总线不需要驱动
    drv_name = "platform_bus driver";
    drv      = new platform_bus_drv_t();
    return;
}

platform_bus_dev_t::~platform_bus_dev_t(void) {
    if (drv != nullptr) {
        delete drv;
    }
    return;
}
