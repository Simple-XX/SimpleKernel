
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

// platform 总线不需要初始化
bool platform_bus_drv_t::init(void) {
    info("platform_bus driver init successful.\n");
    return true;
}
