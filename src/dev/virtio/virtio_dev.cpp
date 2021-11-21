
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_dev.cpp for Simple-XX/SimpleKernel.

#include "virtio_dev.h"

virtio_dev_t::virtio_dev_t(void) {
    return;
}

virtio_dev_t::virtio_dev_t(const resource_t &_resource) : dev_t(_resource) {
// #define DEBUG
#ifdef DEBUG
    std::cout<<"virtio_dev_t ctor."<<std::endl;
#undef DEBUG
#endif
    return;
}

virtio_dev_t::~virtio_dev_t(void) {
    return;
}
