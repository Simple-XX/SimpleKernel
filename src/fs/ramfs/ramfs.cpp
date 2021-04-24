
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// ramfs.cpp for Simple-XX/SimpleKernel.

#include "ramfs.h"
#include "stdlib.h"

RAMFS::RAMFS(void) {
    name  = "ramfs";
    start = malloc(size);
    end   = (uint8_t *)start + size;
    return;
}

RAMFS::~RAMFS(void) {
    return;
}

int RAMFS::get_sb(void) {
    return 0;
}

int RAMFS::kill_sb(void) {
    return 0;
}

int RAMFS::read_super(void) {
    return 0;
}

int RAMFS::open(void) {
    return 0;
}

int RAMFS::close(void) {
    return 0;
}

int RAMFS::seek(void) {
    return 0;
}
