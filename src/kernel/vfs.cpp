
/**
 * @file vfs.cpp
 * @brief 虚拟文件系统实现
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

#include "vfs.h"
#include "iostream"
#include "ramfs.h"
#include "string.h"

superblock_t::superblock_t(void) {
    return;
}

superblock_t::~superblock_t(void) {
    return;
}

inode_t::inode_t(void) {
    return;
}

inode_t::~inode_t(void) {
    return;
}

void inode_t::read(size_t _idx) {
    (void)_idx;
    return;
}

void inode_t::write(size_t _idx) {
    (void)_idx;
    return;
}

dentry_t::dentry_t(void) {
    return;
}

dentry_t::~dentry_t(void) {
    return;
}

file_t::file_t(dentry_t* _dentry, int _flag, fd_t _fd) {
    dentry = _dentry;
    flag   = _flag;
    fd     = _fd;
    return;
}

file_t::~file_t(void) {
    return;
}

FS::FS(void) {
    return;
}

FS::~FS(void) {
    return;
}

VFS::VFS(void) {
    return;
}

VFS::~VFS(void) {
    return;
}

dentry_t* VFS::find_dentry(const mystl::string& _path) {
    // 遍历所有目录项
    for (auto i : dentrys) {
        // 如果完整路径相同
        if (i->path == _path) {
            return i;
        }
    }
    return nullptr;
}

dentry_t* VFS::alloc_dentry(const mystl::string& _path, int _flags) {
    // 新建目录项
    dentry_t* dentry = new dentry_t();
    dentry->flag     = _flags;
    // 找到父目录
    for (auto i : dentrys) {
        // 如果 i 的 ptah 与 _path 的路径部分相同
        if (_path.find(i->path) == 0) {
            // 设置父目录
            dentry->parent = i;
            // 为父目录添加 child
            i->child.push_back(dentry);
        }
    }
    dentry->path      = _path;
    // 设置文件名为 _path 的最后一部分
    mystl::string tmp = _path;
    dentry->name      = tmp.substr(_path.find_last_of('/') + 1);
    // 由路径对应的 fs 分配 inode
    inode_t* inode    = get_fs(_path)->alloc_inode();
    // 将 dentry 与 inode 建立关联
    dentry->inode     = inode;
    dentrys.push_back(dentry);
    return dentry;
}

int VFS::dealloc_dentry(const mystl::string& _path) {
    dentry_t* dentry = find_dentry(_path);
    // 首先判断是否存在，不存在则返回
    if (dentry == nullptr) {
        std::cout << "\"" << _path << "\" not exist." << std::endl;
        return -1;
    }
    FS* fs = get_fs(_path);
    // 从父目录的子目录中删除
    dentry->parent->child.remove(dentry);
    // 删除子目录
    for (auto i : dentry->child) {
        fs->dealloc_inode(i->inode);
        dentrys.remove(i);
    }
    // 删除 inode
    fs->dealloc_inode(dentry->inode);
    // 删除 dentry
    dentrys.remove(dentry);
    return 0;
}

FS* VFS::get_fs(const mystl::string& _path) {
    for (auto i : fs) {
        // 返回 0 说明路径匹配
        if (_path.find(i->root.name) == 0) {
            return i;
        }
    }
    return nullptr;
}

fd_t VFS::alloc_fd(void) {
    return 0;
}

fd_t VFS::dealloc_fd(void) {
    return 0;
}

int32_t VFS::init(void) {
    // 注意内存泄漏
    // 创建 "/" 目录
    // 建立第一个目录项
    dentry_t* dentry = new dentry_t();
    dentry->flag     = 0;
    // 根目录的父目录是自己
    dentry->parent   = dentry;
    dentry->path     = "/";
    dentry->name     = "/";
    // 注册 ramfs 作为 rootfs
    RAMFS* rootfs
      = new RAMFS("rootfs", *dentry, malloc(RAMFS_SIZE), RAMFS_SIZE);
    // 建立第一个 inode
    inode_t* inode = rootfs->alloc_inode();
    // 将 dentry 与 inode 建立关联
    dentry->inode  = inode;
    fs.push_back((FS*)rootfs);
    dentrys.push_back(dentry);
    cwd = dentry;
    std::cout << "vfs init." << std::endl;
    return 0;
}

int32_t VFS::register_filesystem(FS* _fs) {
    // 遍历文件系统
    for (auto i : fs) {
        // 有的话返回
        if (i->name == _fs->name) {
            std::cout << _fs->name << " file system already exist."
                      << std::endl;
            return -1;
        }
        // 没有的话注册
        else {
            fs.push_back(_fs);
            std::cout << "register " << _fs->name << " file system."
                      << std::endl;
            break;
        }
    }
    return 0;
}

int32_t VFS::unregister_filesystem(FS* _fs) {
    // 删除下属的 dentry
    // 如果 i 的 name 与 _fs 的安装路径匹配
    for (auto i : dentrys) {
        if (i->name.find(_fs->root.path) == 0) {
            dentrys.remove(i);
        }
    }
    // 遍历文件系统
    for (auto i : fs) {
        // 有的话删除
        if (i->name == _fs->name) {
            fs.remove(i);
            std::cout << "unregister " << _fs->name << " file system."
                      << std::endl;
            return -1;
        }
    }
    return 0;
}

int VFS::mkdir(const mystl::string& _path, const mode_t& _mode) {
    // TODO: 对 "/", ".", ".." 等特殊字符的处理
    // 首先判断是否存在，存在则返回
    if (find_dentry(_path) != nullptr) {
        std::cout << "\"" << _path << "\" already exist." << std::endl;
        return -1;
    }
    // 没有找到则创建
    dentry_t* dentry = alloc_dentry(_path, _mode);
    if (dentry == nullptr) {
        return -1;
    }
    return 0;
}

int VFS::rmdir(const mystl::string& _path) {
    return dealloc_dentry(_path);
}

int VFS::open(const mystl::string& _path, int _flags) {
    // 首先看是否存在
    dentry_t* dentry = find_dentry(_path);
    // 如果不存在
    if (dentry == nullptr) {
        // 根据 flag 判断是否创建
        // 创建
        if ((_flags & O_CREAT) != 0) {
            dentry = alloc_dentry(_path, _flags);
        }
        // 不创建则返回
        else {
            return -1;
        }
    }
    // 打开
    // 创建 file 对象
    file_t* file = new file_t(dentry, _flags, alloc_fd());
    // 添加到链表中
    files.push_back(file);
    return file->fd;
}

int VFS::close(fd_t _fd) {
    for (auto i : files) {
        if (i->fd == _fd) {
            files.remove(i);
        }
    }
    return 0;
}

int VFS::read(fd_t _fd, void* _buf, size_t _count) {
    // 寻找对应 file
    for (auto i : files) {
        if (i->fd == _fd) {
            memcpy(_buf, i->dentry->inode->pointer, _count);
        }
    }
    return 0;
}

int VFS::write(fd_t _fd, void* _buf, size_t _count) {
    for (auto i : files) {
        if (i->fd == _fd) {
            i->dentry->inode->pointer = malloc(_count);
            memcpy(i->dentry->inode->pointer, _buf, _count);
        }
    }
    return 0;
}

VFS* vfs;
