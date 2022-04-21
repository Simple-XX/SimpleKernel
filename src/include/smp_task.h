
/**
 * @file smp_task.h
 * @brief 任务管理
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-01-22
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2022-01-22<td>MRNIU<td>新建文件
 * </table>
 */

#ifndef _SMP_TASK_H_
#define _SMP_TASK_H_

#include "scheduler.h"
#include "fifo_scheduler.h"
#include "rr_scheduler.h"
#include "task.h"
#include "common.h"
#include "cpu.hpp"
#include "core.h"
#include "spinlock.h"

// 任务管理
// 初始化 调度 pid 开始 退出 等待 etc.
// 调度--调度器
//      获取下一个任务
//      安排任务运行在哪个 cpu 上
//

/**
 * @brief 任务管理与调度
 */
class SMP_TASK {
private:
    /// 自旋锁
    spinlock_t spinlock;
    /// 任务调度器队列
    mystl::vector<scheduler_t *> schedulers;
    /// 全局 pid
    pid_t g_pid;

    /**
     * @brief 获取下一个要运行的任务
     * @return task_t*          下一个要运行的任务
     */
    task_t *get_next_task(void);

    /**
     * @brief 分配 pid
     * @return pid_t            分配出的 pid
     */
    pid_t alloc_pid(void);

    /**
     * @brief 回收 pid
     * @param  _pid             要回收的 pid
     */
    void free_pid(pid_t _pid);

    /**
     * @brief 切换任务
     */
    void switch_task(void);

protected:
public:
    /**
     * @brief 调取器，与 vector 中对应
     * @note 调度优先级 rr==fifo->cfs(->idle)
     */
    enum scheduler_type_t {
        SCHEDULER_RR,
        SCHEDULER_FIFO,
        SCHEDULER_CFS,
    };

    SMP_TASK(void);
    ~SMP_TASK(void);

    /**
     * @brief 获取单例
     * @return SMP_TASK&        静态对象
     */
    static SMP_TASK &get_instance(void);

    /**
     * @brief 初始化
     * @return true             成功
     * @return false            失败
     */
    bool init(void);
    bool init_other_core(void);

    /**
     * @brief 添加一个任务
     * @param  _task            要添加的任务
     * @param  _scheduler_type  此任务使用的调度器
     */
    bool add_task(task_t &_task, scheduler_type_t _scheduler_type);

    /**
     * @brief 删除任务
     * @param  _task            要删除的任务
     */
    void remove_task(task_t &_task);

    /**
     * @brief 调度
     * @note 从内核调度线程切换到其它线程
     */
    void sched(void);

    /**
     * @brief 线程退出
     * @param  _exit_code       退出代码
     */
    void exit(uint32_t _exit_code);
};

#endif /* _SMP_TASK_H_ */
