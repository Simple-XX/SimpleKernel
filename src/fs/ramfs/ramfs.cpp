
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// ramfs.cpp for Simple-XX/SimpleKernel.

#include "ramfs.h"
#include "stdlib.h"

ramfs_inode_t::ramfs_inode_t(void) {
    return;
}

ramfs_inode_t::~ramfs_inode_t(void) {
    return;
}

ramfs_superblock_t::ramfs_superblock_t(void) {
    block_total = 0;
    inode_total = 0;
    block_free  = 0;
    inode_free  = 0;
    block_size  = 0;
    inode_size  = 0;
    return;
}

ramfs_superblock_t::~ramfs_superblock_t(void) {
    return;
}

int ramfs_superblock_t::read(void) {
    return 0;
}

RAMFS::RAMFS(const STL::string &_name, const dentry_t &_dentry) {
    ramfs_superblock_t *super = new ramfs_superblock_t();
    supers.push_back((superblock_t *)super);
    name = _name;
    root = _dentry;
    return;
}

RAMFS::~RAMFS(void) {
    for (auto i : supers) {
        supers.remove(i);
        delete i;
    }
    for (auto i : inodes) {
        inodes.remove(i);
        delete i;
    }
    return;
}

inode_t *RAMFS::alloc_inode(void) {
    inode_t *inode = (inode_t *)new ramfs_inode_t();
    inode->size    = 0;
    // inode.device_id = 0;
    // inode.user_id   = 0;
    // inode.group_id  = 0;
    // inode.mode      = 0;
    // inode.flag      = 0;
    // inode.ctime      = 0;
    // inode.mtime      = 0;
    // inode.atime      = 0;
    inode->hard_links = 1;
    // inode.pointer    = 0;
    inodes.push_back(inode);
    for (auto i : supers) {
        i->inode_total += 1;
    }
    return inode;
}

void RAMFS::dealloc_inode(inode_t *_inode) {
    inodes.remove(_inode);
    delete _inode;
    for (auto i : supers) {
        i->inode_total -= 1;
    }
    return;
}
