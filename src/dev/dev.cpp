
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// dev.cpp for Simple-XX/SimpleKernel.

#include "dev.h"

dev_t::dev_t(const mystl::vector<resource_t> &_res) : res(_res) {
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

dev_t::~dev_t(void) {
    return;
}
