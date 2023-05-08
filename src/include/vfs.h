
/**
 * @file vfs.h
 * @brief 虚拟文件系统头文件
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

#ifndef SIMPLEKERNEL_VFS_H
#define SIMPLEKERNEL_VFS_H

#include "list"
#include "stdint.h"
#include "string"
#include "time.h"

typedef uint32_t                device_id_t;
typedef uint32_t                user_id_t;
typedef uint32_t                group_id_t;
typedef uint32_t                mode_t;
typedef uint32_t                flag_t;
typedef uint32_t                fd_t;

// open for reading only
static constexpr const uint32_t O_RDONLY   = 0x0000;
// open for writing only
static constexpr const uint32_t O_WRONLY   = 0x0001;
// open for reading and writing
static constexpr const uint32_t O_RDWR     = 0x0002;
// mask for above modes
static constexpr const uint32_t O_ACCMODE  = 0x0003;
// no delay
static constexpr const uint32_t O_NONBLOCK = 0x0004;
// set append mode
static constexpr const uint32_t O_APPEND   = 0x0008;
// create if nonexistant
static constexpr const uint32_t O_CREAT    = 0x0200;
// truncate to zero length
static constexpr const uint32_t O_TRUNC    = 0x0400;
// error if already exists
static constexpr const uint32_t O_EXCL     = 0x0800;

// 超级块
// fs 挂载时从存储介质读入
class superblock_t {
private:

protected:

public:
    superblock_t(void);
    virtual ~superblock_t(void) = 0;
    // 读写
    virtual int read(void)      = 0;
    virtual int write(void)     = 0;
};

// inode 索引节点
class inode_t {
private:

protected:

public:
    // 文件大小 以字节为单位表示的
    size_t      size;
    // 设备 ID，标识容纳该文件的设备。
    device_id_t device_id;
    // 文件所有者的 User ID。
    user_id_t   user_id;
    // 文件的 Group ID
    group_id_t  group_id;
    // 文件的模式（mode），确定了文件的类型，以及它的所有者、它的group、其它用户访问此文件的权限。
    mode_t      mode;
    // 额外的系统与用户标志（flag），用来保护该文件。
    flag_t      flag;
    // inode 被修改的时间。
    time_t      ctime;
    // 文件内容被修改的时间。
    time_t      mtime;
    // 最后一次访问的时间。
    time_t      atime;
    // 1个链接数，表示有多少个硬链接指向此inode。
    uint32_t    hard_links;
    // 到文件系统存储位置的指针。通常是1K字节或者2K字节的存储容量为基本单位。
    void*       pointer;
    inode_t(void);
    ~inode_t(void);
    void read(size_t _idx);
    void write(size_t _idx);
};

// 目录项
class dentry_t {
private:

protected:

public:
    uint32_t               flag;
    // 对应的 inode
    inode_t*               inode;
    // 父目录
    dentry_t*              parent;
    // 子目录
    mystl::list<dentry_t*> child;
    // 文件完整路径
    mystl::string          path;
    // 文件名
    mystl::string          name;
    dentry_t(void);
    virtual ~dentry_t(void);
};

// 文件
class file_t {
private:

protected:

public:
    // 文件描述符
    fd_t      fd;
    uint32_t  flag;
    // 偏移量
    size_t    offset;
    // 对应的 dentry
    dentry_t* dentry;
    file_t(dentry_t* _dentry, int _flag, fd_t _fd);
    virtual ~file_t(void);
};

// 实际的文件系统
class FS {
private:

protected:
    // 超级块
    mystl::list<superblock_t*> supers;
    // inode 链表
    mystl::list<inode_t*>      inodes;

public:
    // 文件系统名
    mystl::string name;
    // 挂载点目录项
    dentry_t      root;
    FS(void);
    virtual ~FS(void)                               = 0;
    // 分配 inode
    virtual inode_t* alloc_inode(void)              = 0;
    virtual void     dealloc_inode(inode_t* _inode) = 0;
};

// 所有文件系统由 vfs 统一管理
// 操作文件时由 vfs 使用实际文件系统进行操作
class VFS {
private:
    // 管理的文件系统
    mystl::list<FS*>       fs;
    // denty 链表
    mystl::list<dentry_t*> dentrys;
    // 所有打开的文件
    mystl::list<file_t*>   files;
    // 当前所在目录
    dentry_t*              cwd;
    // 查找目录项
    dentry_t*              find_dentry(const mystl::string& _path);
    // 新建目录项
    dentry_t*              alloc_dentry(const mystl::string& _path, int _flags);
    // 删除目录项
    int                    dealloc_dentry(const mystl::string& _path);
    // 根据路径判断文件系统
    FS*                    get_fs(const mystl::string& _path);
    // 分配文件描述符
    fd_t                   alloc_fd(void);
    fd_t                   dealloc_fd(void);

protected:

public:
    VFS(void);
    ~VFS(void);
    /**
     * @brief 获取单例
     * @return VFS&             静态对象
     */
    static VFS& get_instance(void);

    /**
     * @brief 初始化
     * @return true            成功
     * @return false           失败
     * @todo 移动到构造函数去
     */
    bool        init(void);

    // 添加一个文件系统
    int32_t     register_filesystem(FS* _fs);
    // 删除一个文件系统
    int32_t     unregister_filesystem(FS* _fs);
    // 文件系统相关
    //  挂载  设备名，挂载路径，文件系统名
    int mount(const mystl::string& _dev_name, const mystl::string& _path,
              const mystl::string& _fs_name, unsigned long flags, void* data);
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
    // 创建目录
    int mkdir(const mystl::string& _path, const mode_t& _mode);
    // 删除目录及其内容
    int rmdir(const mystl::string& _path);
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
    // 打开文件
    int open(const mystl::string& _path, int _flags);
    int close(fd_t _fd);
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
    int read(fd_t _fd, void* _buf, size_t _count);
    int write(fd_t _fd, void* _buf, size_t _count);
    int readv(void);
    int writev(void);
    int sendfile(void);
    int sendfile64(void);
    int readahead(void);
};

// VFS 依赖 C++ 库，不能在 cpp_init 中初始化
extern VFS* vfs;

#endif /* SIMPLEKERNEL_VFS_H */
