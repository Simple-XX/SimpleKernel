
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

/**
 * @brief 调度器抽象类
 */
class SCHEDULER {
private:
protected:
    /// 调度器名
    const char *name;

    /// 任务向量
    mystl::queue<task_t *> *task_queue;

    /// 全局 pid
    pid_t g_pid;

    /// 自旋锁
    spinlock_t spinlock;

    /**
     * @brief 分配 pid
     * @return pid_t            分配出的 pid
     */
    virtual pid_t alloc_pid(void) = 0;

    /**
     * @brief 回收 pid
     * @param  _pid             要回收的 pid
     */
    virtual void free_pid(pid_t _pid) = 0;

    /**
     * @brief 获取下一个要运行的任务
     * @return task_t*          下一个要运行的任务
     */
    virtual task_t *get_next_task(void) = 0;

    /**
     * @brief 切换任务
     */
    virtual void switch_task(void)=0;

public:
    /**
     * @brief 调度器构造
     * @param  _name            调度器名
     */
    SCHEDULER(const char *_name);

    virtual ~SCHEDULER(void) = 0;

    /**
     * @brief 添加一个任务
     * @param  _task            要添加的任务
     */
    virtual void add_task(task_t *_task) = 0;

    /**
     * @brief 删除任务
     * @param  _task            要删除的任务
     */
    virtual void remove_task(task_t *_task) = 0;
};

#endif /* _SCHEDULER_H_ */
