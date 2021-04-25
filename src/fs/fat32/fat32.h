
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// fat32.h for Simple-XX/SimpleKernel.

#ifndef _FAT32_H_
#define _FAT32_H_

#include "vfs.h"

class FAT32 : FS {
private:
    // inode 数量
    static constexpr const uint32_t inode_size = 0x20000000;
    // 超级块
    // inode 索引

protected:
public:
    FAT32(void);
    ~FAT32(void);
    int get_sb(void);
    int kill_sb(void);
    int read_super(void);
    int open();
    int close();
    int seek();
};

#endif /* _FAT32_H_ */
