
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// vfs.cpp for Simple-XX/SimpleKernel.

#include "iostream"
#include "stdint.h"
#include "cstring.h"
#include "common.h"
#include "pmm.h"
#include "ramfs.h"
#include "vfs.h"

inode_t::inode_t(void) {
    return;
}

inode_t::~inode_t(void) {
    return;
}

superblock_t::superblock_t(void) {
    return;
}

superblock_t::~superblock_t(void) {
    return;
}

dentry_t::dentry_t(void) {
    return;
}

dentry_t::~dentry_t(void) {
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

dentry_t *VFS::find_dentry(const STL::string &_path) {
    // 遍历所有目录项
    for (auto i : dentrys) {
        // 如果完整路径相同
        if (i->path == _path) {
            return i;
        }
    }
    return nullptr;
}

FS *VFS::get_fs(const STL::string &_path) {
    for (auto i : fs) {
        // 返回 0 说明路径匹配
        if (_path.find(i->root.name) == 0) {
            return i;
        }
    }
    return nullptr;
}

int32_t VFS::init(void) {
    // 注意内存泄漏
    // 创建 "/" 目录
    // 建立第一个目录项
    dentry_t *dentry = new dentry_t();
    dentry->flag     = 0;
    // 根目录的父目录是自己
    dentry->parent = dentry;
    dentry->path   = "/";
    dentry->name   = "/";
    // 注册 ramfs 作为 rootfs
    RAMFS *rootfs = new RAMFS("rootfs", *dentry);
    // 建立第一个 inode
    inode_t *inode = rootfs->alloc_inode();
    // 将 dentry 与 inode 建立关联
    dentry->inode = inode;
    fs.push_back((FS *)rootfs);
    dentrys.push_back(dentry);
    cwd = dentry;
    std::cout << "vfs init." << std::endl;
    return 0;
}

int32_t VFS::register_filesystem(FS *_fs) {
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

int32_t VFS::unregister_filesystem(FS *_fs) {
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
            // TODO: 确保 delete 调用了析构函数
            delete i;
            std::cout << "unregister " << _fs->name << " file system."
                      << std::endl;
            return -1;
        }
    }
    return 0;
}

int VFS::mkdir(const STL::string &_path, const mode_t &_mode) {
    // TODO: 对 "/", ".", ".." 等特殊字符的处理
    // 首先判断是否存在，存在则返回
    if (find_dentry(_path) != nullptr) {
        std::cout << "\"" << _path << "\" already exist." << std::endl;
        return -1;
    }
    // 没有找到则创建
    // 新建目录项
    dentry_t *dentry = new dentry_t();
    dentry->flag     = 0;
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
    dentry->path = _path;
    // 设置文件名为 _path 的最后一部分
    dentry->name = _path.substr(_path.find_last_of('/') + 1);
    // 由路径对应的 fs 分配 inode
    inode_t *inode = get_fs(_path)->alloc_inode();
    // 将 dentry 与 inode 建立关联
    dentry->inode = inode;
    dentrys.push_back(dentry);
    std::cout << "par: " << dentry->parent->path << ", name: " << dentry->name
              << ", path: " << dentry->path << std::endl;
    return 0;
}

int VFS::rmdir(const STL::string &_path) {
    dentry_t *dentry = find_dentry(_path);
    FS *      fs     = get_fs(_path);
    // 首先判断是否存在，不存在则返回
    if (dentry == nullptr) {
        std::cout << "\"" << _path << "\" not exist." << std::endl;
        return -1;
    }
    // 设置父目录
    dentry->parent->child.remove(dentry);
    // 删除子目录
    for (auto i : dentry->child) {
        fs->dealloc_inode(i->inode);
        dentrys.remove(i);
        delete i;
    }
    // 删除 inode
    fs->dealloc_inode(dentry->inode);
    // 删除 dentry
    dentrys.remove(dentry);
    delete dentry;
    return 0;
}

int VFS::open(const STL::string &_path, int _flags) {

    return 0;
}

int VFS::close(int _fd) {
    return 0;
}
