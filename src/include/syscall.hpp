/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cpu_io.h>

#include <cstddef>
#include <cstdint>

#include "signal.hpp"

// 参考 Linux 系统调用号
#if defined(__riscv) || defined(__aarch64__)
// RISC-V 64 和 AArch64 使用 asm-generic 编号
inline constexpr uint64_t kSyscallWrite = 64;
inline constexpr uint64_t kSyscallExit = 93;
inline constexpr uint64_t kSyscallYield = 124;
inline constexpr uint64_t kSyscallClone = 220;
inline constexpr uint64_t kSyscallGettid = 178;
inline constexpr uint64_t kSyscallFutex = 98;
inline constexpr uint64_t kSyscallSetTidAddress = 96;
inline constexpr uint64_t kSyscallFork = 1220;
inline constexpr uint64_t kSyscallSleep = 101;
inline constexpr uint64_t kSyscallKill = 129;
inline constexpr uint64_t kSyscallSigaction = 134;
inline constexpr uint64_t kSyscallSigprocmask = 135;
inline constexpr uint64_t kSyscallSchedGetaffinity = 123;
inline constexpr uint64_t kSyscallSchedSetaffinity = 122;
#else
#error "Unsupported architecture for syscall numbers"
#endif

// 由各个架构实现
auto Syscall(uint64_t cause, cpu_io::TrapContext* context) -> void;

auto syscall_dispatcher(int64_t syscall_id, uint64_t args[6]) -> int;

/**
 * @brief 向文件描述符写入数据
 * @param fd 文件描述符
 * @param buf 数据缓冲区
 * @param len 数据长度
 * @return 成功写入的字节数，失败返回负数
 * @note 使用场景：标准输出、日志输出等
 */
[[nodiscard]] auto sys_write(int fd, const char* buf, size_t len) -> int;

/**
 * @brief 退出当前进程或线程
 * @param code 退出码
 * @return 不返回
 * @note 使用场景：
 *       - 进程正常/异常终止
 *       - 线程退出（clone 创建的线程调用时只退出当前线程）
 * @details 行为取决于线程创建方式：
 *          - 普通进程：退出整个进程
 *          - CLONE_THREAD 线程：仅退出当前线程
 */
auto sys_exit(int code) -> int;

/**
 * @brief 主动放弃CPU，让出时间片
 * @return 0 表示成功
 * @note 使用场景：协作式调度、忙等待优化
 */
[[nodiscard]] auto sys_yield() -> int;

/**
 * @brief 休眠指定毫秒数
 * @param ms 休眠时长（毫秒）
 * @return 0 表示成功
 * @note 使用场景：定时任务、延迟执行
 */
[[nodiscard]] auto sys_sleep(uint64_t ms) -> int;

/**
 * @brief 创建新线程（或进程）
 * @param flags 克隆标志（CLONE_VM、CLONE_THREAD、CLONE_SIGHAND 等）
 * @param stack 新线程的栈指针（用户空间）
 * @param parent_tid 父线程TID存储地址（可选）
 * @param child_tid 子线程TID存储地址（可选）
 * @param tls 线程本地存储指针
 * @return 成功返回新线程TID，失败返回负数
 * @note 使用场景：
 *       - pthread_create 底层实现
 *       - fork() 实现（不带 CLONE_VM）
 *       - 创建轻量级线程
 * @details flags 常用组合：
 *          - CLONE_VM | CLONE_THREAD | CLONE_SIGHAND: 创建共享地址空间的线程
 *          - 0: 创建独立进程（类似 fork）
 */
[[nodiscard]] auto sys_clone(uint64_t flags, void* stack, int* parent_tid,
                             int* child_tid, void* tls) -> int;

/**
 * @brief 创建新进程（fork）
 * @return 父进程返回子进程 PID，子进程返回 0，失败返回 -1
 * @note 使用场景：
 *       - 进程创建
 *       - 完全复制父进程的地址空间和资源
 * @details fork 创建的子进程：
 *          - 拥有独立的地址空间（页表被复制）
 *          - 拥有独立的进程 ID (PID)
 *          - 继承父进程的文件描述符、信号处理器等
 *          - 父子进程独立运行，互不影响
 */
[[nodiscard]] auto sys_fork() -> int;

/**
 * @brief 获取当前线程ID
 * @return 当前线程的TID
 * @note 使用场景：
 *       - 线程标识
 *       - 调试信息输出
 *       - 线程本地数据索引
 */
[[nodiscard]] auto sys_gettid() -> int;

/**
 * @brief 设置线程ID地址（用于线程退出时的清理）
 * @param tidptr 线程ID存储地址
 * @return 成功返回0，失败返回负数
 * @note 使用场景：
 *       - 线程库内部使用
 *       - 实现线程退出时的通知机制
 */
[[nodiscard]] auto sys_set_tid_address(int* tidptr) -> int;

/**
 * @brief 快速用户空间互斥锁操作（futex）
 * @param uaddr 用户空间的futex地址
 * @param op 操作类型（FUTEX_WAIT、FUTEX_WAKE 等）
 * @param val 操作参数
 * @param timeout 超时时间（可选）
 * @param uaddr2 第二个futex地址（部分操作使用）
 * @param val3 第三个参数（部分操作使用）
 * @return 依操作类型而定，失败返回负数
 * @note 使用场景：
 *       - 实现互斥锁（mutex）
 *       - 实现条件变量（condition variable）
 *       - 实现读写锁（rwlock）
 *       - 实现信号量（semaphore）
 *       - 实现 pthread_join（等待线程退出）
 *       - 实现 pthread_detach（线程状态管理）
 * @details 常用操作：
 *          - FUTEX_WAIT: 等待futex值变化
 *          - FUTEX_WAKE: 唤醒等待的线程
 *          - FUTEX_REQUEUE: 重新排队等待的线程
 */
[[nodiscard]] auto sys_futex(int* uaddr, int op, int val, const void* timeout,
                             int* uaddr2, int val3) -> int;

/**
 * @brief 获取线程的CPU亲和性
 * @param pid 线程ID（0 表示当前线程）
 * @param cpusetsize CPU集合大小
 * @param mask CPU亲和性掩码
 * @return 成功返回 0，失败返回负数
 */
[[nodiscard]] auto sys_sched_getaffinity(int pid, size_t cpusetsize,
                                         uint64_t* mask) -> int;

/**
 * @brief 设置线程的CPU亲和性
 * @param pid 线程ID（0 表示当前线程）
 * @param cpusetsize CPU集合大小
 * @param mask CPU亲和性掩码
 * @return 成功返回 0，失败返回负数
 * @note 使用场景：
 *       - CPU绑定优化
 *       - 实时任务调度
 *       - NUMA优化
 */
[[nodiscard]] auto sys_sched_setaffinity(int pid, size_t cpusetsize,
                                         const uint64_t* mask) -> int;

/**
 * @brief 向指定进程发送信号
 * @param pid 目标进程 PID
 * @param sig 信号编号
 * @return 成功返回 0，失败返回负数
 */
[[nodiscard]] auto sys_kill(int pid, int sig) -> int;

/**
 * @brief 设置信号处理函数
 * @param signum 信号编号
 * @param handler 新的信号处理函数
 * @return 成功返回 0，失败返回负数
 */
[[nodiscard]] auto sys_sigaction(int signum, SignalHandler handler) -> int;

/**
 * @brief 修改进程信号掩码
 * @param how 操作方式 (SIG_BLOCK, SIG_UNBLOCK, SIG_SETMASK)
 * @param set 要操作的信号集
 * @param oldset 旧信号集存储位置（可为 nullptr）
 * @return 成功返回 0，失败返回负数
 */
[[nodiscard]] auto sys_sigprocmask(int how, uint32_t set, uint32_t* oldset)
    -> int;
