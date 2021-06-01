
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// ramfs.h for Simple-XX/SimpleKernel.

#ifndef _RAMFS_H_
#define _RAMFS_H_

#include "string"
#include "vfs.h"

class ramfs_inode_t : inode_t {
private:
protected:
public:
    ramfs_inode_t(void);
    ~ramfs_inode_t(void);
};

class ramfs_superblock_t : superblock_t {
private:
protected:
public:
    ramfs_superblock_t(void);
    ~ramfs_superblock_t(void);
    // 读取
    int read(void);
    int write(void);
};

class RAMFS : FS {
private:
protected:
public:
    RAMFS(const mystl::string &_name, const dentry_t &_dentry);
    ~RAMFS(void);
    inode_t *alloc_inode(void);
    void     dealloc_inode(inode_t *_inode);
};

#endif /* _RAMFS_H_ */
