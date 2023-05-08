
/**
 * @file ramfs.cpp
 * @brief ram 文件系统实现
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-05-08
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-09-18<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#include "ramfs.h"

ramfs_superblock_t::ramfs_superblock_t(void) {
    return;
}

ramfs_superblock_t::~ramfs_superblock_t(void) {
    return;
}

int ramfs_superblock_t::read(void) {
    return 0;
}

int ramfs_superblock_t::write(void) {
    return 0;
}

RAMFS::RAMFS(const mystl::string& _name, const dentry_t& _dentry, void* _start,
             size_t _size) {
    ramfs_superblock_t* super = (ramfs_superblock_t*)_start;
    super->start_addr         = _start;
    super->total              = _size;
    super->sector_size        = SECTOR_SIZE;
    super->sectors            = _size / SECTOR_SIZE;
    super->inode_sector       = INODE_SECTOR;
    super->inode_count        = INODE_COUNT;
    super->data_sector        = DATA_SECTOR;
    super->data_length        = DATA_LENGTH;
    supers.push_back((superblock_t*)super);
    // 添加 inode 到 inodes
    // inode 的开始地址为 super 结束
    // 结束地址为 _start+super大小+inode大小
    uint8_t* tmp = (uint8_t*)_start + super->sector_size;
    for (; tmp < (uint8_t*)_start + super->sector_size * SUPER_LENGTH
                   + super->inode_count * sizeof(inode_t);
         tmp += sizeof(inode_t)) {
        inodes.push_back((inode_t*)tmp);
    }
    name = _name;
    root = _dentry;
    return;
}

RAMFS::~RAMFS(void) {
    for (auto i : supers) {
        supers.remove(i);
    }
    for (auto i : inodes) {
        inodes.remove(i);
    }
    return;
}

inode_t* RAMFS::alloc_inode(void) {
    inode_t* inode    = new inode_t();
    inode->size       = 0;
    // inode.device_id = 0;
    // inode.user_id   = 0;
    // inode.group_id  = 0;
    // inode.mode      = 0;
    // inode.flag      = 0;
    // inode.ctime      = 0;
    // inode.mtime      = 0;
    // inode.atime      = 0;
    inode->hard_links = 1;
    inode->pointer    = nullptr;
    inodes.push_back(inode);
    return inode;
}

void RAMFS::dealloc_inode(inode_t* _inode) {
    inodes.remove(_inode);
    return;
}
