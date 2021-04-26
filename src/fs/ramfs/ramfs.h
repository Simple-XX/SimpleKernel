
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// ramfs.h for Simple-XX/SimpleKernel.

#ifndef _RAMFS_H_
#define _RAMFS_H_

#include "string"
#include "vfs.h"
#include "list.hpp"

class ramfs_superblock_t : superblock_t {
private:
protected:
public:
    ramfs_superblock_t(void);
    ~ramfs_superblock_t(void);
    // 读取
    int read(void);
};

class RAMFS : FS {
private:
protected:
public:
    RAMFS(const STL::string _name);
    ~RAMFS(void);
    inode_t *alloc_inode(void);
    int      mkdir(const STL::string _path, const mode_t _mode);
};

#endif /* _RAMFS_H_ */
