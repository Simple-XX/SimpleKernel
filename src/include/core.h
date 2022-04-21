
/**
 * @file core.h
 * @brief core 抽象
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

#ifndef _CORE_H_
#define _CORE_H_

#include "stdint.h"
#include "stddef.h"
#include "common.h"
#include "task.h"
#include "iostream"

/**
 * @brief core 抽象
 */
struct core_t {
    /// 当前 core id
    size_t core_id;
    /// 当前此 core 上运行的进程
    task_t *curr_task;
    /// 调度线程
    task_t *sched_task;
    /// 中断嵌套深度
    ssize_t noff;
    /// 在进入调度线程前是否允许中断
    bool intr_enable;
    /// 所有 core 数组
    static core_t cores[COMMON::CORES_COUNT];

    core_t(void);

    /**
     * @brief 设置当前 core 正在运行的线程
     * @param  _task            My Param doc
     */
    static void set_curr_task(task_t *_task);

    /**
     * @brief 获取当前 core 正在运行的线程
     * @return task_t*          当前 core 正在运行的线程
     */
    static task_t *get_curr_task(void);

    friend std::ostream &operator<<(std::ostream &_os, const core_t &_core);
};

#endif /* _CORE_H_ */
