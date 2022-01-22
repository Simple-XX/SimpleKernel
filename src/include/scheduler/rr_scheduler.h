
/**
 * @file rr_scheduler.h
 * @brief round-robin 调度器
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

#ifndef _RR_SCHEDULER_H_
#define _RR_SCHEDULER_H_

#include "string"
#include "scheduler.h"
#include "stack"

class rr_scheduler_t : scheduler_t {
private:
protected:
public:
    rr_scheduler_t(void);
    rr_scheduler_t(const mystl::string _name);
    ~rr_scheduler_t(void);

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
     * @brief 获取下一个要运行的任务
     * @return task_t*          下一个要运行的任务
     */
    task_t *get_next_task(void) override;
};

#endif /* _RR_SCHEDULER_H_ */
