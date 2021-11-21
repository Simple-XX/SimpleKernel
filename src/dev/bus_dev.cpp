
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// bus_dev.cpp for Simple-XX/SimpleKernel.

#include "bus_dev.h"

bus_dev_t::bus_dev_t(void) {
    return;
}

bus_dev_t::bus_dev_t(const resource_t &_resource) : dev_t(_resource) {
// #define DEBUG
#ifdef DEBUG
    printf("bus_dev_t ctor.\n");
#undef DEBUG
#endif
    return;
}

bus_dev_t::~bus_dev_t(void) {
    return;
}

bool bus_dev_t::add_drv(drv_t *_drv) {
    drvs.push_back(_drv);
    return true;
}

bool bus_dev_t::add_dev(dev_t *_dev) {
    devs.push_back(_dev);
    return true;
}

bool bus_dev_t::match(dev_t &_dev, drv_t &_drv) {
    if (_dev.drv_name == _drv.name) {
        // 设置驱动
        /// @todo 这里应该新建一个驱动对象
        _dev.drv = &_drv;
        return true;
    }
    else {
        return false;
    }
}

void bus_dev_t::show(void) const {
    info("dev count: 0x%X\n", devs.size());
    for (auto i : devs) {
        std::cout << *i << std::endl;
    }
    info("drv count: 0x%X\n", drvs.size());
    for (auto i : drvs) {
        std::cout << *i << std::endl;
    }
    return;
}