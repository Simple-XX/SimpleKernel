
/**
 * @file scheduler.h
 * @brief 调度器抽象头文件
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

#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "stdint.h"
#include "stdio.h"
#include "string"
#include "iostream"
#include "vector"
#include "queue"
#include "task.h"
#include "spinlock.h"

// 调度器抽象
/**
 * @brief 调度器
 */
class SCHEDULER {
private:
    /**
     * @brief 获取下一个要运行的任务
     * @return task_t*          下一个要运行的任务
     */
    static task_t *get_next_task(void);

    /**
     * @brief 切换任务
     */
    static void switch_task(void);

protected:
public:
    /// 当前任务
    static task_t *curr_task[COMMON::CORES_COUNT];

    /// 任务向量
    static mystl::queue<task_t *> *task_queue;

    /// 全局 pid
    static pid_t g_pid;

    /// 内核任务
    static task_t *task_os[COMMON::CORES_COUNT];

    /**
     * @brief 调度
     * @note 从内核调度线程切换到其它线程
     */
    static void sched(void);

    /**
     * @brief 分配 pid
     * @return pid_t            分配出的 pid
     */
    static pid_t alloc_pid(void);

    /**
     * @brief 回收 pid
     * @param  _pid             要回收的 pid
     */
    static void free_pid(pid_t _pid);

    /**
     * @brief 初始化
     * @return true             成功
     * @return false            失败
     */
    static bool init(void);
    static bool init_other_core(void);

    /**
     * @brief 获取当前 core 正在执行的任务
     * @return task_t*
     */
    static task_t *get_curr_task(void);

    /**
     * @brief 添加一个任务
     * @param  _task            要添加的任务
     */
    static void add_task(task_t *_task);

    // 删除任务
    /**
     * @brief 删除任务
     * @param  _task            要删除的任务
     */
    static void rm_task(task_t *_task);

    /**
     * @brief 切换到内核调度线程
     */
    static void switch_to_kernel(void);

    /**
     * @brief 线程退出
     * @param  _exit_code       退出代码
     */
    static void exit(uint32_t _exit_code);
};

#endif /* _SCHEDULER_H_ */
