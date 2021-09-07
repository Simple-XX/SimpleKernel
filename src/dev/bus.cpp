
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// bus.cpp for Simple-XX/SimpleKernel.

#include "bus.h"

bus_t::bus_t(void) {
    return;
}

bus_t::bus_t(const resource_t &_resource) : dev_t(_resource) {
// #define DEBUG
#ifdef DEBUG
    printf("bus_t ctor.\n");
#undef DEBUG
#endif
    return;
}

bus_t::~bus_t(void) {
    return;
}
