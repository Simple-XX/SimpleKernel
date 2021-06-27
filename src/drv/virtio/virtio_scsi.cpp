
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_scsi.cpp for Simple-XX/SimpleKernel.

#include "stdio.h"
#include "virtio_scsi.h"
#include "virtio_queue.h"

VIRTIO_SCSI::VIRTIO_SCSI(void *_addr) : VIRTIO(_addr, SCSI_HOST) {
    printf("virtio scsi init\n");
    return;
}

VIRTIO_SCSI::~VIRTIO_SCSI(void) {
    return;
}
