
/**
 * @file spinlock.h
 * @brief 自旋锁定义
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-01-01
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-01-01<td>MRNIU<td>迁移到 doxygen
 * </table>
 */

#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#include "stddef.h"

/**
 * @brief 自旋锁
 * @note 只能用于多核
 */
struct spinlock_t {
private:
    /// 自旋锁名称
    const char *name;
    /// 是否 lock
    volatile bool locked;
    /// 获得此锁的 hartid
    volatile size_t hartid;

    /**
     * @brief 检查当前 hart 是否获得此锁
     * @return true             是
     * @return false            否
     */
    bool is_holding(void);

    /**
     * @brief 中断嵌套+1
     */
    void push_off(void);

    /**
     * @brief 中断嵌套-1
     */
    void pop_off(void);

public:
    spinlock_t(void);
    spinlock_t(const char *_name);

    /**
     * @brief 获得锁
     */
    void lock(void);

    /**
     * @brief 释放锁
     */
    void unlock(void);
};

#endif /* _SPINLOCK_H_ */
