
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// vfs.cpp for Simple-XX/SimpleKernel.

#include "stdint.h"
#include "cstring.h"
#include "common.h"
#include "pmm.h"
#include "ramfs.h"
#include "vfs.h"

SUPERBLOCK::SUPERBLOCK(void) {
    return;
}

SUPERBLOCK::~SUPERBLOCK(void) {
    return;
}

INODE::INODE(void) {
    return;
}

INODE::~INODE(void) {
    return;
}

FS::FS(void) {
    return;
}

FS::~FS(void) {
    return;
}

VFS::VFS(void) {
    return;
}

VFS::~VFS(void) {
    return;
}

int32_t VFS::init(void) {
    fs.push_back((FS *)&ramfs);
    return 0;
}

int32_t VFS::mount(FS *_fs) {
    fs.push_back(_fs);
    return 0;
}
