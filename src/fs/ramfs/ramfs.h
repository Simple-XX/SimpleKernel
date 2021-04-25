
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// ramfs.h for Simple-XX/SimpleKernel.

#ifndef _RAMFS_H_
#define _RAMFS_H_

#include "vfs.h"

class RAMFS : FS {
private:
    // inode 数量
    static constexpr const uint32_t inode_size = 0x20000000;
    // 超级块
    // inode 索引

protected:
public:
    RAMFS(void);
    ~RAMFS(void);
    // 挂载
    int mount(void);
    // 卸载
    int unmount(void);
    int read_super(void);
    int open();
    int close();
    int seek();
};

#endif /* _RAMFS_H_ */
