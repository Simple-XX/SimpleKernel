
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// dev.cpp for Simple-XX/SimpleKernel.

#include "dev.h"
#include "virtio_blk.h"
#include "virtio_scsi.h"
#include "dtb.h"
#include "string.h"
#include "intr.h"

dev_t::dev_t(void) {
    return;
}

dev_t::~dev_t(void) {
    return;
}

bool dev_t::match_drv(drv_t &_drv) {
    if (_drv.name == drv_name) {
        return true;
    }
    else {
        return false;
    }
}