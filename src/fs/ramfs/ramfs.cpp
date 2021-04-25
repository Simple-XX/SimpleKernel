
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// ramfs.cpp for Simple-XX/SimpleKernel.

#include "ramfs.h"
#include "stdlib.h"

RAMFS::RAMFS(void) {
    name = "ramfs";
    return;
}

RAMFS::~RAMFS(void) {
    return;
}

int RAMFS::mount(void) {
    return 0;
}

int RAMFS::unmount(void) {
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
