
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// platform_bus_dev.cpp for Simple-XX/SimpleKernel.

#include "platform_bus_dev.h"

platform_bus_dev_t::platform_bus_dev_t(void) {
    drv_name = "platform_bus driver";
    return;
}

platform_bus_dev_t::platform_bus_dev_t(const resource_t &_resource)
    : bus_t(_resource) {
    drv_name = "platform_bus driver";
    return;
}

platform_bus_dev_t::~platform_bus_dev_t(void) {
    return;
}
