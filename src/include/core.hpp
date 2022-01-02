
/**
 * @file core.hpp
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

#ifndef _CORE_HPP_
#define _CORE_HPP_

#include "stdint.h"
#include "stddef.h"
#include "common.h"
#include "task.h"

/**
 * @brief core 抽象
 */
struct core_t {
    /// 当前 core id
    ssize_t core_id;
    /// 当前此 core 上运行的进程
    task_t *curr_task;
    /// 调度线程
    task_t *sched_task;
    /// 中断嵌套深度
    ssize_t noff;
    /// 在进入调度线程前是否允许中断
    bool intr_enable;

    core_t(void) {
        core_id     = -1;
        curr_task   = nullptr;
        sched_task  = nullptr;
        noff        = 0;
        intr_enable = false;
        return;
    }
    core_t(size_t _core_id) {
        core_id     = _core_id;
        curr_task   = nullptr;
        sched_task  = nullptr;
        noff        = 0;
        intr_enable = false;
    }
};

static core_t cores[COMMON::CORES_COUNT];

#endif /* _CORE_HPP_ */
