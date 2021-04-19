
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// vfs.h for Simple-XX/SimpleKernel.

#ifndef _VFS_H_
#define _VFS_H_

#include "stdint.h"
#include "io.h"

// 超级块
// fs 挂载时从存储介质读入
class SUPERBLOCK {
private:
    static IO io;

    // block与inode的总量
    uint32_t block_total;
    uint32_t inode_total;
    // 未使用与已使用的inode/block数量
    uint32_t block_free;
    uint32_t inode_free;
    // block与inode的大小（block为1K,2K,4K；inode为128byte）
    static constexpr const uint32_t BLOCK_SIZE = 0x1000;
    static constexpr const uint32_t INODE_SIZE = 0x80;
    // 文件系统的挂在时间、最近一次写入数据的时间、最近一次检验磁盘的时间等文件系统的相关信息
    // 一个validbit数值，若此文件系统已被挂载，则validbit为0，否则为1
    bool valid;
    // 文件系统描述--File system Description
    // 块对应表--block bitmap 记录磁盘中使用与未使用的block号码
    // inode对应表--inode bitmap 记录使用与未使用的inode号码

protected:
public:
    SUPERBLOCK(void);
    ~SUPERBLOCK(void);
};

// inode 索引节点
class INODE {
private:
    static IO io;

    // 该文件的访问模式(read/write/excute)
    uint32_t flag;
    // 该文件的所有者与组(owner/group)
    uint32_t owner;
    // 该文件的大小
    uint64_t size;
    // 指向此文件内容的硬链接数
    uint32_t hl;
    // 该文件创建或状态改变的时间(ctime)
    uint32_t ctime;
    // 最近一次的读取时间(atime)
    uint32_t atime;
    // 最近修改的时间(mtime)
    uint32_t mtime;
    // 定义文件特性的标志(flag)，如SetUID等
    // 该文件真正内容的指向(point)
    uint32_t point;

protected:
public:
    INODE(void);
    ~INODE(void);
} __attribute__((aligned(128)));

// 文件
class FILE {
private:
    static IO io;
    uint32_t  flag;

protected:
public:
    FILE(void);
    virtual ~FILE(void);
    virtual int open();
    virtual int close();
    virtual int seek();
};

class VFS {
private:
    static IO         io;
    static SUPERBLOCK superblock;

protected:
public:
    VFS(void);
    ~VFS(void);
};

#endif /* _VFS_H_ */
