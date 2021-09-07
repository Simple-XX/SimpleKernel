
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// dev.cpp for Simple-XX/SimpleKernel.

#include "dev.h"

dev_t::dev_t(void) {
    dev_name = "dev_t";
    drv_name = "";
    bus_name = "";
    drv      = nullptr;
// #define DEBUG
#ifdef DEBUG
    printf("dev_t ctor.\n");
#undef DEBUG
#endif
    return;
}

dev_t::dev_t(const resource_t &_resource) : resource(_resource) {
    dev_name = resource.name;
    drv_name = "";
    bus_name = "";
    drv      = nullptr;
// #define DEBUG
#ifdef DEBUG
    printf("dev_t ctor.\n");
#undef DEBUG
#endif
    return;
}

dev_t::~dev_t(void) {
    return;
}
