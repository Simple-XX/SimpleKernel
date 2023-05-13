
/**
 * @file heap.h
 * @brief 堆抽象头文件
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

#ifndef SIMPLEKERNEL_HEAP_H
#define SIMPLEKERNEL_HEAP_H

#include "allocator.h"
#include "cstddef"
#include "cstdint"
#include "slab.h"

/**
 * @brief 堆抽象
 */
class HEAP {
private:
    // 堆分配器
    ALLOCATOR* allocator_kernel;
    ALLOCATOR* allocator_non_kernel;

protected:

public:
    /**
     * @brief 获取单例
     * @return HEAP&            静态对象
     */
    static HEAP& get_instance(void);

    /** 初始化
     * @brief 堆初始化
     * @return true            成功
     * @return false           失败
     */
    bool         init(void);

    /**
     * @brief 内核地址内存申请
     * @param  _byte           要申请的 bytes
     * @return void*           申请到的地址
     */
    void*        kmalloc(size_t _byte);

    /**
     * @brief 内核地址内存释放
     * @param  _p              要释放的内存地址
     */
    void         kfree(void* _p);

    /**
     * @brief 内存申请
     * @param  _byte           要申请的 bytes
     * @return void*           申请到的地址
     */
    void*        malloc(size_t _byte);

    /**
     * @brief 内存释放
     * @param  _p              要释放的内存地址
     */
    void         free(void* _p);
};

#endif /* SIMPLEKERNEL_HEAP_H */
