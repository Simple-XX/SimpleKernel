
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

int32_t VFS::init(void) {
    // 注册 ramfs 作为 rootfs
    RAMFS *rootfs = new RAMFS("rootfs");
    fs.push_back((FS *)rootfs);
    // 创建 "/" 目录
    mkdir("/", 0);
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

int VFS::mkdir(STL::string _path, mode_t _mode) {
    FS *path_fs;
    // 首先判断是否存在，存在则返回
    // 需要一个函数解析文件路径
    // 再判断所属文件系统
    // 最后调用实际文件系统创建
    // 在目录树中查找 path，判断 path 所属文件系统
    for (auto i : fs) {
        // 找到则返回
        if (false) {
            std::cout << _path << " already exist." << std::endl;
            return -1;
        }
        path_fs = i;
    }
    // 没有找到则创建
    return path_fs->mkdir(_path, _mode);
}
