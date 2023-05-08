
/**
 * @file ramfs.h
 * @brief ram 文件系统头文件
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

#ifndef SIMPLEKERNEL_RAMFS_H
#define SIMPLEKERNEL_RAMFS_H

#include "string"
#include "vfs.h"

// ramfs 结构
//      超级块 索引节点 数据
// 扇区    0    1~3   4~end

// ramfs 大小 16 MB
static constexpr const size_t RAMFS_SIZE = 0x1000000;

class ramfs_superblock_t : superblock_t {
private:

protected:

public:
    // 开始地址
    void*  start_addr;
    // 总大小
    size_t total;
    // 扇区大小
    size_t sector_size;
    // 总扇区数
    size_t sectors;
    // inode 表开始扇区
    size_t inode_sector;
    // inode 个数
    size_t inode_count;
    // 数据区开始扇区
    size_t data_sector;
    // 数据区长度
    size_t data_length;
    ramfs_superblock_t(void);
    ~ramfs_superblock_t(void);
    // 读写
    int read(void);
    int write(void);
};

// 簇
class ramfs_cluster_t {
private:

protected:

public:
    ramfs_cluster_t(void);
    ~ramfs_cluster_t(void);
    // 读写
    void read(size_t _idx);
    void write(size_t _idx);
};

class RAMFS : FS {
private:
    // 扇区大小 512B
    static constexpr const size_t SECTOR_SIZE  = 0x200;
    // 簇大小 4KB
    static constexpr const size_t CLUSTER_SIZE = 0x1000;
    // 超级块开始扇区
    static constexpr const size_t SUPER_SECTOR = 0x0;
    // 超级块长度(扇区)
    static constexpr const size_t SUPER_LENGTH = 0x1;
    // inode 开始扇区
    static constexpr const size_t INODE_SECTOR = 1;
    // inode 个数
    static constexpr const size_t INODE_COUNT  = 0x80;
    // 数据区开始扇区
    static constexpr const size_t DATA_SECTOR  = 0x4;
    // 数据区长度(扇区) = 总大小 - 超级块占用的扇区大小 - inode 占用的扇区大小
    static constexpr const size_t DATA_LENGTH
      = RAMFS_SIZE - SUPER_LENGTH * SECTOR_SIZE - INODE_COUNT * SECTOR_SIZE;

protected:

public:
    // _name: 文件系统名
    // _dentry: 挂载的目录
    // _start: 内存开始
    // _size: 长度
    // 在构造函数中初始化 ramfs 的相关信息
    RAMFS(const mystl::string& _name, const dentry_t& _dentry, void* _start,
          size_t _size);
    ~RAMFS(void);
    inode_t* alloc_inode(void);
    void     dealloc_inode(inode_t* _inode);
    // 读写扇区
    void     read_sector(size_t _num);
    void     write_sector(size_t _num);
};

#endif /* SIMPLEKERNEL_RAMFS_H */
