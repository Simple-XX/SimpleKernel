
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
class SCHEDULER {
private:
    static task_t *get_next_task(void);
    static void    switch_task(void);

protected:
public:
    static void sched(void);
    // 当前任务
    static task_t *curr_task[COMMON::CORES_COUNT];
    // 任务向量
    static mystl::queue<task_t *> *task_queue;
    // 全局 pid
    static pid_t   g_pid;
    static task_t *task_os[COMMON::CORES_COUNT];
    // 分配 pid
    static pid_t alloc_pid(void);
    // 回收 pid
    static void free_pid(pid_t _pid);
    // 初始化
    static bool init(void);
    static bool init_other_core(void);
    // 获取当前任务
    static task_t *get_curr_task(void);
    // 添加任务
    static void add_task(task_t *_task);
    // 删除任务
    static void rm_task(task_t *_task);
    static void switch_to_kernel(void);
};

#endif /* _SCHEDULER_H_ */
