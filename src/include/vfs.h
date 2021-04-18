
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// vfs.h for Simple-XX/SimpleKernel.

#ifndef _VFS_H_
#define _VFS_H_

#include "stdint.h"
#include "io.h"

// 超级块
class SUPERBLOCK {
private:
    static IO io;

protected:
public:
    SUPERBLOCK(void);
    ~SUPERBLOCK(void);
};

// inode 索引节点
class INODE {
private:
    static IO io;

protected:
public:
    INODE(void);
    ~INODE(void);
};

// 文件
class FILE {
private:
    static IO io;

protected:
public:
    FILE(void);
    ~FILE(void);
};

// 目录
class DENTRY {
private:
    static IO io;

protected:
public:
    DENTRY(void);
    ~DENTRY(void);
};

class VFS {
private:
    static IO io;

protected:
public:
    VFS(void);
    ~VFS(void);
};

#endif /* _VFS_H_ */
