
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
#include "string"
#include "atomic"

/**
 * @brief 自旋锁
 * @note 只能用于多核
 */
struct spinlock_t {
private:
    /// 自旋锁名称
    const char *name;
    /// 是否 lock
    std::atomic_flag locked;
    /// 获得此锁的 hartid
    size_t hartid;

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

    /**
     * @brief 构造函数
     * @param  _name            锁名
     * @note 需要堆初始化后可用
     */
    spinlock_t(const char *_name);

    /**
     * @brief 构造函数
     * @param  _name            锁名
     * @note 需要 libcxx 始化后可用
     */
    spinlock_t(const mystl::string &_name);

    /**
     * @brief 初始化，用于堆完成之前的初始化
     * @param  _name            锁名称
     * @return true             成功
     * @return false            失败
     */
    bool init(const char *_name);

    /**
     * @brief 获得锁
     */
    void lock(void);

    /**
     * @brief 释放锁
     */
    void unlock(void);

    friend std::ostream &operator<<(std::ostream     &_os,
                                    const spinlock_t &_spinlock) {
        printf("spinlock(%s) hart 0x%X %s\n", _spinlock.name, _spinlock.hartid,
               (_spinlock.locked._M_i ? "locked" : "unlock"));
        return _os;
    }
};

#endif /* _SPINLOCK_H_ */
