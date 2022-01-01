
/**
 * @file task.h
 * @brief 任务抽象头文件
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

#ifndef _TASK_H_
#define _TASK_H_

#include "stdint.h"
#include "types.h"
#include "stdio.h"
#include "string"
#include "iostream"
#include "vector"
#include "queue"
#include "spinlock.h"
#include "vmm.h"
#include "cpu.hpp"

/**
 * @brief 任务状态
 */
enum task_status_t : uint8_t {
    /// 未使用
    UNUSED,
    /// 睡眠
    SLEEPING,
    /// 可运行
    RUNNABLE,
    /// 运行中
    RUNNING,
    /// 僵尸进程
    ZOMBIE,
};

/**
 * @brief 任务抽象
 */
struct task_t {
    /// 任务名
    mystl::string name;
    // 任务 id
    pid_t pid;
    // 进程状态
    enum task_status_t state;
    // 父进程
    task_t *parent;
    // 上下文
    CPU::context_t context;
    // 线程栈
    uintptr_t stack;
    // 进程页目录
    pt_t page_dir;
    // 本次运行时间片
    size_t slice;
    // 已运行时间片
    size_t slice_total;
    size_t hartid;
    // Depth of push_off() nesting.
    int noff = 0;
    // Were interrupts enabled before push_off()?
    bool is_intr_enable;
    task_t(void);
    task_t(mystl::string _name, pid_t _pid, void (*_task)(void));
    ~task_t(void);
    // 退出
    void exit(uint32_t _exit_code);
    // task_t 输出
    friend std::ostream &operator<<(std::ostream &_os, const task_t &_task);
};

#endif /* _TASK_H_ */
