
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// bus.cpp for Simple-XX/SimpleKernel.

#include "bus.h"

bus_t::bus_t(const mystl::vector<resource_t *> &_res) : dev_t(_res) {
    return;
}

bus_t::~bus_t(void) {
    return;
}
