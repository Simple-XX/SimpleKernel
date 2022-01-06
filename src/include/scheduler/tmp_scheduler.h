
/**
 * @file tmp_scheduler.h
 * @brief 临时调度器抽象头文件
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

#ifndef _tmp_SCHEDULER_H_
#define _tmp_SCHEDULER_H_

#include "stdint.h"
#include "stdio.h"
#include "string"
#include "iostream"
#include "vector"
#include "queue"
#include "task.h"
#include "spinlock.h"
#include "scheduler.h"

/**
 * @brief 调度器
 */
class tmp_SCHEDULER : SCHEDULER {
private:
    /// 当前任务
    static task_t *curr_task[COMMON::CORES_COUNT];

    /// 内核任务
    static task_t *task_os[COMMON::CORES_COUNT];

    /**
     * @brief 分配 pid
     * @return pid_t            分配出的 pid
     */
    pid_t alloc_pid(void) override;

    /**
     * @brief 回收 pid
     * @param  _pid             要回收的 pid
     */
    void free_pid(pid_t _pid) override;

    /**
     * @brief 获取下一个要运行的任务
     * @return task_t*          下一个要运行的任务
     */
    task_t *get_next_task(void) override;

    /**
     * @brief 切换任务
     */
    void switch_task(void) override;

protected:
public:
    tmp_SCHEDULER(void);
    ~tmp_SCHEDULER(void);

    /**
     * @brief 获取单例
     * @return tmp_SCHEDULER&       静态对象
     */
    static tmp_SCHEDULER &get_instance(void);

    /**
     * @brief 初始化
     * @return true             成功
     * @return false            失败
     */
    bool init(void);
    bool init_other_core(void);

    /**
     * @brief 调度
     * @note 从内核调度线程切换到其它线程
     */
    void sched(void);

    /**
     * @brief 获取当前 core 正在执行的任务
     * @return task_t*
     */
    task_t *get_curr_task(void);

    /**
     * @brief 添加一个任务
     * @param  _task            要添加的任务
     */
    void add_task(task_t *_task) override;

    /**
     * @brief 删除任务
     * @param  _task            要删除的任务
     */
    void remove_task(task_t *_task) override;

    /**
     * @brief 切换到内核调度线程
     */
    static void switch_to_kernel(void);

    /**
     * @brief 线程退出
     * @param  _exit_code       退出代码
     */
    void exit(uint32_t _exit_code);
};

#endif /* _tmp_SCHEDULER_H_ */
