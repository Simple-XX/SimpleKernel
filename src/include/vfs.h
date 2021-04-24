
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// vfs.h for Simple-XX/SimpleKernel.

#ifndef _VFS_H_
#define _VFS_H_

#include "stdint.h"
#include "string"
#include "time.h"

// 超级块
// fs 挂载时从存储介质读入
class SUPERBLOCK {
private:
    // block与inode的总量
    uint32_t block_total;
    uint32_t inode_total;
    // 未使用与已使用的inode/block数量
    uint32_t block_free;
    uint32_t inode_free;
    // block与inode的大小（block为1K,2K,4K；inode为128byte）
    uint32_t block_size;
    uint32_t inocde_size;
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

typedef uint32_t device_id_t;
typedef uint32_t user_id_t;
typedef uint32_t group_id_t;
typedef uint32_t mode_t;
typedef uint32_t flag_t;

// inode 索引节点
class INODE {
private:
    // 以字节为单位表示的文件大小。
    size_t bytes;
    // 设备 ID，标识容纳该文件的设备。
    device_id_t device_id;
    // 文件所有者的 User ID。
    user_id_t user_id;
    // 文件的 Group ID
    group_id_t group_id;
    // 文件的模式（mode），确定了文件的类型，以及它的所有者、它的group、其它用户访问此文件的权限。
    mode_t mode;
    // 额外的系统与用户标志（flag），用来保护该文件。
    flag_t flag;
    // inode 被修改的时间。
    time_t ctime;
    // 文件内容被修改的时间。
    time_t mtime;
    // 最后一次访问的时间。
    time_t atime;
    // 1个链接数，表示有多少个硬链接指向此inode。
    uint32_t hard_links;
    // 到文件系统存储位置的指针。通常是1K字节或者2K字节的存储容量为基本单位。
    uint32_t point;

protected:
public:
    INODE(void);
    ~INODE(void);
} __attribute__((aligned(128)));

// 目录
class DENTRY {
private:
    uint32_t flag;

protected:
public:
    DENTRY(void);
    virtual ~DENTRY(void);
    virtual int open();
    virtual int close();
    virtual int seek();
};

// 文件
class FILE {
private:
    uint32_t flag;

protected:
public:
    FILE(void);
    virtual ~FILE(void);
    virtual int open();
    virtual int close();
    virtual int seek();
};

// 实际的文件系统
class FS {
private:
    // 超级块
    SUPERBLOCK *supers[4];

protected:
public:
    FS(void);
    // 文件系统名
    STL::string name;
    virtual ~FS(void) = 0;
    // 初始化超级块
    virtual int get_sb(void) = 0;
    // 删除超级块
    virtual int kill_sb(void) = 0;
    // 读取超级块
    virtual int read_super(void) = 0;
    virtual int open(void)       = 0;
    virtual int close(void)      = 0;
    virtual int seek(void)       = 0;
};

// 所有文件系统由 vfs 统一管理
// 文件系统的注册
// 操作文件时由 vfs 使用实际文件系统进行操作
class VFS {
private:
    // 管理的文件系统
    FS *fs[4];

protected:
public:
    VFS(void);
    ~VFS(void);
    // 初始化根安装点
    int32_t init(void);
    // 添加一个文件系统
    //文件系统相关
    int32_t mount(FS *_fs);
    int32_t umount(void);
    int32_t umount2(void);
    int32_t sysfs(void);
    int32_t statfs(void);
    int32_t fstatfs(void);
    int32_t fstatfs64(void);
    int32_t ustat(void);
    // 目录相关
    int32_t chroot(void);
    int32_t pivot_root(void);
    int32_t chdir(void);
    int32_t fchdir(void);
    int32_t getcwd(void);
    int32_t mkdir(void);
    int32_t rmdir(void);
    int32_t getdents(void);
    int32_t getdents64(void);
    int32_t readdir(void);
    int32_t link(void);
    int32_t unlink(void);
    int32_t rename(void);
    int32_t lookup_dcookie(void);
    // 链接相关
    int32_t readlink(void);
    int32_t symlink(void);
    // 文件相关
    int32_t chown(void);
    int32_t fchown(void);
    int32_t lchown(void);
    int32_t chown16(void);
    int32_t fchown16(void);
    int32_t lchown16(void);
    int32_t hmod(void);
    int32_t fchmod(void);
    int32_t utime(void);
    int32_t stat(void);
    int32_t fstat(void);
    int32_t lstat(void);
    int32_t acess(void);
    int32_t oldstat(void);
    int32_t oldfstat(void);
    int32_t oldlstat(void);
    int32_t stat64(void);
    int32_t lstat64(void);
    // int32_t lstat64(void);
    int32_t open(void);
    int32_t close(void);
    int32_t creat(void);
    int32_t umask(void);
    int32_t dup(void);
    int32_t dup2(void);
    int32_t fcntl(void);
    int32_t fcntl64(void);
    int32_t select(void);
    int32_t poll(void);
    int32_t truncate(void);
    int32_t ftruncate(void);
    int32_t truncate64(void);
    int32_t ftruncate64(void);
    int32_t lseek(void);
    int32_t llseek(void);
    int32_t read(void);
    int32_t write(void);
    int32_t readv(void);
    int32_t writev(void);
    int32_t sendfile(void);
    int32_t sendfile64(void);
    int32_t readahead(void);
};

static VFS vfs;

#endif /* _VFS_H_ */
