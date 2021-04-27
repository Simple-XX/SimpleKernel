
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// ramfs.cpp for Simple-XX/SimpleKernel.

#include "ramfs.h"
#include "stdlib.h"

ramfs_superblock_t::ramfs_superblock_t(void) {
    block_total = 0;
    inode_total = 0;
    block_free  = 0;
    inode_free  = 0;
    // 4kb
    block_size = 0x1000;
    // 128B
    inode_size = 0x80;
    return;
}

ramfs_superblock_t::~ramfs_superblock_t(void) {
    return;
}

int ramfs_superblock_t::read(void) {
    return 0;
}

RAMFS::RAMFS(const STL::string _name) {
    name                      = _name;
    ramfs_superblock_t *super = new ramfs_superblock_t();
    supers.push_back((superblock_t *)super);
    return;
}

RAMFS::~RAMFS(void) {
    return;
}

inode_t *RAMFS::alloc_inode(void) {
    inode_t *inode = new inode_t();
    // inode.size       = 0;
    // inode.device_id  = 0;
    // inode.user_id    = 0;
    // inode.group_id   = 0;
    // inode.mode       = 0;
    // inode.flag       = 0;
    // inode.ctime      = 0;
    // inode.mtime      = 0;
    // inode.atime      = 0;
    inode->hard_links = 1;
    // inode.pointer    = 0;
    inodes.push_back(inode);
    return inode;
}

dentry_t *RAMFS::alloc_denty(void) {
    dentry_t *dentry = new dentry_t();
    dentrys.push_back(dentry);
    return dentry;
}

int RAMFS::mkdir(const STL::string _path, const mode_t _mode) {
    // 这里传递过来的路径确保是不存在的
    // 新建 inode
    inode_t *inode = alloc_inode();
    // 新建 dentry
    dentry_t *dentry = alloc_denty();
    // 更新其它数据
    return 0;
}
