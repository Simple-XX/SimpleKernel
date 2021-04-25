
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// vfs.h for Simple-XX/SimpleKernel.

#ifndef _VFS_H_
#define _VFS_H_

#include "stdint.h"
#include "string"
#include "vector"
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
    virtual ~SUPERBLOCK(void) = 0;
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
    virtual ~INODE(void);
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
    STL::vector<SUPERBLOCK *> supers;

protected:
public:
    FS(void);
    // 文件系统名
    STL::string name;
    virtual ~FS(void) = 0;
    // 挂载
    virtual int mount(void) = 0;
    // 卸载
    virtual int unmount(void) = 0;
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
    STL::vector<FS *> fs;

protected:
public:
    VFS(void);
    ~VFS(void);
    // 初始化根安装点
    int32_t init(void);
    //文件系统相关
    // 添加一个文件系统
    int32_t register_filesystem(FS *_fs);
    // 挂载
    int mount(const char *source, const char *target,
              const char *filesystemtype, unsigned long mountflags,
              const void *data);
    int umount(void);
    int umount2(void);
    int sysfs(void);
    int statfs(void);
    int fstatfs(void);
    int fstatfs64(void);
    int ustat(void);
    // 目录相关
    int chroot(void);
    int pivot_root(void);
    int chdir(void);
    int fchdir(void);
    int getcwd(void);
    int mkdir(void);
    int rmdir(void);
    int getdents(void);
    int getdents64(void);
    int readdir(void);
    int link(void);
    int unlink(void);
    int rename(void);
    int lookup_dcookie(void);
    // 链接相关
    int readlink(void);
    int symlink(void);
    // 文件相关
    int chown(void);
    int fchown(void);
    int lchown(void);
    int chown16(void);
    int fchown16(void);
    int lchown16(void);
    int hmod(void);
    int fchmod(void);
    int utime(void);
    int stat(void);
    int fstat(void);
    int lstat(void);
    int acess(void);
    int oldstat(void);
    int oldfstat(void);
    int oldlstat(void);
    int stat64(void);
    int lstat64(void);
    int open(const char *path, int flags);
    int close(void);
    int creat(void);
    int umask(void);
    int dup(void);
    int dup2(void);
    int fcntl(void);
    int fcntl64(void);
    int select(void);
    int poll(void);
    int truncate(void);
    int ftruncate(void);
    int truncate64(void);
    int ftruncate64(void);
    int lseek(void);
    int llseek(void);
    int read(void);
    int write(void);
    int readv(void);
    int writev(void);
    int sendfile(void);
    int sendfile64(void);
    int readahead(void);
};

static VFS vfs;

#endif /* _VFS_H_ */
