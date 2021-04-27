
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// vfs.h for Simple-XX/SimpleKernel.

#ifndef _VFS_H_
#define _VFS_H_

#include "stdint.h"
#include "string"
#include "list.hpp"
#include "time.h"

typedef uint32_t device_id_t;
typedef uint32_t user_id_t;
typedef uint32_t group_id_t;
typedef uint32_t mode_t;
typedef uint32_t flag_t;

// inode 索引节点
class inode_t {
private:
protected:
public:
    // 文件大小 以字节为单位表示的
    size_t size;
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
    uint32_t pointer;
    inode_t(void);
    virtual ~inode_t(void);
};

// 超级块
// fs 挂载时从存储介质读入
class superblock_t {
private:
protected:
public:
    // inode 与 block 的总量
    uint32_t inode_total;
    uint32_t block_total;
    // 未使用的 inode 与 block 数量
    uint32_t inode_free;
    uint32_t block_free;
    // block与inode的大小（block为1K,2K,4K；inode为128byte）
    uint32_t block_size;
    uint32_t inode_size;
    // 文件系统的挂在时间、最近一次写入数据的时间、最近一次检验磁盘的时间等文件系统的相关信息
    // 文件系统描述--File system Description
    // 块对应表--block bitmap 记录磁盘中使用与未使用的block号码
    // inode对应表--inode bitmap 记录使用与未使用的inode号码
    superblock_t(void);
    virtual ~superblock_t(void) = 0;
    // 读取
    virtual int read(void) = 0;
};

// 目录
class dentry_t {
private:
protected:
public:
    uint32_t flag;
    // 对应的 inode
    inode_t *inode;
    // 父目录
    dentry_t *parent;
    // 子目录
    STL::list<dentry_t *> child;
    // 文件名
    STL::string name;
    dentry_t(void);
    virtual ~dentry_t(void);
};

// 实际的文件系统
class FS {
private:
protected:
    // 超级块
    STL::list<superblock_t *> supers;
    // inode 链表
    STL::list<inode_t *> inodes;
    // denty 链表
    STL::list<dentry_t *> dentrys;
    // 分配 inode
    virtual inode_t *alloc_inode(void) = 0;
    // 分配 denty
    virtual dentry_t *alloc_denty(void) = 0;

public:
    // 文件系统名
    STL::string name;
    FS(void);
    virtual ~FS(void)                                              = 0;
    virtual int mkdir(const STL::string _path, const mode_t _mode) = 0;
};

// 所有文件系统由 vfs 统一管理
// 操作文件时由 vfs 使用实际文件系统进行操作
class VFS {
private:
    // 管理的文件系统
    STL::list<FS *> fs;
    // 根目录
    inode_t root;

protected:
public:
    VFS(void);
    ~VFS(void);
    // 初始化根安装点
    int32_t init(void);
    // 添加一个文件系统
    int32_t register_filesystem(FS *_fs);
    // 删除一个文件系统
    int32_t unregister_filesystem(FS *_fs);
    //文件系统相关
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
    int mkdir(STL::string _path, mode_t _mode);
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

// 文件
class file_t {
private:
protected:
public:
    uint32_t flag;
    file_t(void);
    virtual ~file_t(void);
};

static VFS vfs;

#endif /* _VFS_H_ */
