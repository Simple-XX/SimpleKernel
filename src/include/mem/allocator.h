
/**
 * @file allocator.h
 * @brief 分配器抽象类头文件
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-09-18<td>digmouse233<td>迁移到 doxygen
 * </table>
 */

#ifndef SIMPLEKERNEL_ALLOCATOR_H
#define SIMPLEKERNEL_ALLOCATOR_H

#include "stddef.h"
#include "stdint.h"

/**
 * @brief 内存分配器抽象类
 */
class ALLOCATOR {
private:
protected:
    /// 分配器名称
    const char *name;
    /// 当前管理的内存区域地址
    uintptr_t allocator_start_addr;
    /// 当前管理的内存区域长度
    size_t allocator_length;
    /// 当前管理的内存区域空闲长度
    size_t allocator_free_count;
    /// 当前管理的内存区域已使用长度
    size_t allocator_used_count;

public:
    /**
     * @brief 构造内存分配器
     * @param  _name           分配器名
     * @param  _addr           要管理的内存开始地址
     * @param  _len            要管理的内存长度，单位以具体实现为准
     */
    ALLOCATOR(const char *_name, uintptr_t _addr, size_t _len);

    virtual ~ALLOCATOR(void) = 0;

    /**
     * @brief 分配 _len 页
     * @param  _len            页数
     * @return uintptr_t       分配到的地址
     */
    virtual uintptr_t alloc(size_t _len) = 0;

    /**
     * @brief 在指定地址分配 _len 长度
     * @param  _addr           指定的地址
     * @param  _len            长度
     * @return true            成功
     * @return false           失败
     */
    virtual bool alloc(uintptr_t _addr, size_t _len) = 0;

    /**
     * @brief 释放 _len 长度
     * @param  _addr           地址
     * @param  _len            长度
     */
    virtual void free(uintptr_t _addr, size_t _len) = 0;

    /**
     * @brief 已使用数量
     * @return size_t          数量
     */
    virtual size_t get_used_count(void) const = 0;

    /**
     * @brief 空闲数量
     * @return size_t          数量
     */
    virtual size_t get_free_count(void) const = 0;
};

#endif /* SIMPLEKERNEL_ALLOCATOR_H */
