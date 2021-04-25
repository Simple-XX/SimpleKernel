
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// vfs.cpp for Simple-XX/SimpleKernel.

#include "stdio.h"
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
    RAMFS ramfs;
    register_filesystem((FS *)&ramfs);
    printf("vfs init.\n");
    return 0;
}

int32_t VFS::register_filesystem(FS *_fs) {
    // 遍历文件系统
    for (auto i : fs) {
        // 有的话返回
        if (i->name == _fs->name) {
            return -1;
        }
        // 没有的话注册
        else {
            fs.push_back(_fs);
            break;
        }
    }
    return 0;
}
