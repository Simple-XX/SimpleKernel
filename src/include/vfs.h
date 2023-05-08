
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
 * <tr><td>2023-05-08<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#ifndef SIMPLEKERNEL_VFS_H
#define SIMPLEKERNEL_VFS_H

#include "cstdint"
#include "list"
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

/**
 * @brief inode 索引节点 index node
 * 保存在磁盘上
 */
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

/**
 * @brief 目录项 directory entry
 * 保存在内存中，由内核维护
 */
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

// 实际的文件系统
class fs_t {
private:

protected:

public:
    // 文件系统名
    mystl::string name;
    // 挂载点目录项
    dentry_t      root;
    fs_t(void);
    virtual ~fs_t(void)                             = 0;
    // 分配 inode
    virtual inode_t* alloc_inode(void)              = 0;
    virtual void     dealloc_inode(inode_t* _inode) = 0;
};

/**
 * @brief 虚拟文件系统
 * 所有文件系统由 vfs 统一管理
 * 操作文件时由 vfs 使用实际文件系统进行操作
 */
class VFS {
private:
    /// 管理的文件系统
    mystl::list<fs_t*>     fs;
    /// dentry_t 缓存
    mystl::list<dentry_t*> dentries;
    /// inode_t 缓存
    mystl::list<inode_t*>   inodes;
    /// 当前所在目录
    mystl::string          pwd;
    /// 查找目录项
    dentry_t*              find_dentry(const mystl::string& _path);
    /// 新建目录项
    dentry_t*              alloc_dentry(const mystl::string& _path, int _flags);
    /// 删除目录项
    int                    dealloc_dentry(const mystl::string& _path);
    /// 根据路径判断文件系统
    fs_t*                  get_fs(const mystl::string& _path);
    /// 分配文件描述符
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
    int32_t     register_filesystem(fs_t* _fs);
    // 删除一个文件系统
    int32_t     unregister_filesystem(fs_t* _fs);
    // 文件系统相关
    //  挂载  设备名，挂载路径，文件系统名
    int32_t
            mount(const mystl::string& _dev_name, const mystl::string& _path,
                  const mystl::string& _fs_name, unsigned long flags, void* data);
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
    // 创建目录
    int32_t mkdir(const mystl::string& _path, const mode_t& _mode);
    // 删除目录及其内容
    int32_t rmdir(const mystl::string& _path);
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
    // 打开文件
    int32_t open(const mystl::string& _path, int _flags);
    int32_t close(fd_t _fd);
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
    int32_t read(fd_t _fd, void* _buf, size_t _count);
    int32_t write(fd_t _fd, void* _buf, size_t _count);
    int32_t readv(void);
    int32_t writev(void);
    int32_t sendfile(void);
    int32_t sendfile64(void);
    int32_t readahead(void);
};

#endif /* SIMPLEKERNEL_VFS_H */
