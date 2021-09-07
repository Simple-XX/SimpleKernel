
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// platform_bus_drv.cpp for Simple-XX/SimpleKernel.

#include "platform_bus_drv.h"

platform_bus_drv_t::platform_bus_drv_t(void) {
    name = "platform_bus driver";
    return;
}

platform_bus_drv_t::~platform_bus_drv_t(void) {
    return;
}

bool platform_bus_drv_t::init(void) {
    // 遍历资源列表，为每个 resource_t 进行初始化
    info("platform_bus driver init successful.\n");
    return true;
}

void platform_bus_drv_t::read(void) {
    return;
}

void platform_bus_drv_t::write(void) {
    return;
}
