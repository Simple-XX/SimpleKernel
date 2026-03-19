/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <array>
#include <atomic>
#include <cassert>
#include <cstdint>

/// 最大信号数
inline constexpr uint8_t kMaxSignals = 32;

/**
 * @brief 信号编号 (参考 POSIX 标准)
 */
namespace signal_number {
/// 无信号
inline constexpr int kSigNone = 0;
/// 终端挂起
inline constexpr int kSigHup = 1;
/// 键盘中断 (Ctrl+C)
inline constexpr int kSigInt = 2;
/// 键盘退出 (Ctrl+\)
inline constexpr int kSigQuit = 3;
/// 非法指令
inline constexpr int kSigIll = 4;
/// 异常终止
inline constexpr int kSigAbrt = 6;
/// 浮点异常
inline constexpr int kSigFpe = 8;
/// 强制终止 (不可捕获)
inline constexpr int kSigKill = 9;
/// 段错误
inline constexpr int kSigSegv = 11;
/// 管道破裂
inline constexpr int kSigPipe = 13;
/// 定时器到期
inline constexpr int kSigAlrm = 14;
/// 终止请求
inline constexpr int kSigTerm = 15;
/// 子进程状态变化
inline constexpr int kSigChld = 17;
/// 继续 (如果已停止)
inline constexpr int kSigCont = 18;
/// 停止 (不可捕获)
inline constexpr int kSigStop = 19;
/// 终端停止 (Ctrl+Z)
inline constexpr int kSigTstp = 20;
/// 用户自定义信号 1
inline constexpr int kSigUsr1 = 10;
/// 用户自定义信号 2
inline constexpr int kSigUsr2 = 12;
}  // namespace signal_number

/**
 * @brief 信号掩码操作
 */
namespace signal_mask_op {
/// 添加到信号掩码
inline constexpr int kSigBlock = 0;
/// 从信号掩码移除
inline constexpr int kSigUnblock = 1;
/// 设置信号掩码
inline constexpr int kSigSetmask = 2;
}  // namespace signal_mask_op

/// 信号处理函数类型
using SignalHandler = void (*)(int);

/// 默认信号处理 (终止进程)
inline constexpr SignalHandler kSigDfl = nullptr;

/// 忽略信号
inline const SignalHandler kSigIgn = reinterpret_cast<SignalHandler>(1);

/**
 * @brief 信号动作结构
 *
 * 定义每个信号的处理方式。简化版的 POSIX sigaction。
 */
struct SignalAction {
  /// 信号处理函数
  SignalHandler handler{kSigDfl};
  /// 执行信号处理时阻塞的信号集
  uint32_t mask{0};

  /// @name 构造/析构函数
  /// @{
  SignalAction() = default;
  SignalAction(const SignalAction&) = default;
  SignalAction(SignalAction&&) = default;
  auto operator=(const SignalAction&) -> SignalAction& = default;
  auto operator=(SignalAction&&) -> SignalAction& = default;
  ~SignalAction() = default;
  /// @}
};

/**
 * @brief 每个任务的信号状态
 *
 * 管理信号的投递、屏蔽和处理器注册。
 */
struct SignalState {
  /// 待处理信号位掩码
  std::atomic<uint32_t> pending{0};
  /// 阻塞信号位掩码
  std::atomic<uint32_t> blocked{0};
  /// 每个信号的处理动作
  std::array<SignalAction, kMaxSignals> actions{};

  /**
   * @brief 检查是否有可投递的信号
   * @return true 有未屏蔽的待处理信号
   */
  [[nodiscard]] auto HasDeliverableSignal() const -> bool {
    return (pending.load(std::memory_order_acquire) &
            ~blocked.load(std::memory_order_acquire)) != 0;
  }

  /**
   * @brief 获取下一个可投递的信号编号
   * @return int 信号编号，无可投递信号返回 0
   * @note 优先级从低编号到高编号
   */
  [[nodiscard]] auto GetNextDeliverableSignal() const -> int {
    uint32_t deliverable = pending.load(std::memory_order_acquire) &
                           ~blocked.load(std::memory_order_acquire);
    if (deliverable == 0) {
      return signal_number::kSigNone;
    }
    // 返回最低位的信号
    for (int i = 1; i < kMaxSignals; ++i) {
      if (deliverable & (1U << i)) {
        return i;
      }
    }
    return signal_number::kSigNone;
  }

  /**
   * @brief 设置待处理信号
   * @param signum 信号编号
   */
  auto SetPending(int signum) -> void {
    assert(IsValid(signum) && "signum is invalid");
    pending.fetch_or(1U << signum, std::memory_order_release);
  }

  /**
   * @brief 清除待处理信号
   * @param signum 信号编号
   */
  auto ClearPending(int signum) -> void {
    assert(IsValid(signum) && "signum is invalid");
    pending.fetch_and(~(1U << signum), std::memory_order_release);
  }

  /**
   * @brief 检查信号是否不可捕获 (SIGKILL, SIGSTOP)
   * @param signum 信号编号
   * @return true 信号不可捕获
   */
  [[nodiscard]] static constexpr auto IsUncatchable(int signum) -> bool {
    return signum == signal_number::kSigKill ||
           signum == signal_number::kSigStop;
  }

  /**
   * @brief 检查信号编号是否有效
   * @param signum 信号编号
   * @return true 有效
   */
  [[nodiscard]] static constexpr auto IsValid(int signum) -> bool {
    return signum > 0 && signum < kMaxSignals;
  }

  /**
   * @brief 获取信号名称（调试用）
   * @param signum 信号编号
   * @return 信号名称字符串
   */
  [[nodiscard]] static constexpr auto GetSignalName(int signum) -> const char* {
    switch (signum) {
      case signal_number::kSigHup:
        return "SIGHUP";
      case signal_number::kSigInt:
        return "SIGINT";
      case signal_number::kSigQuit:
        return "SIGQUIT";
      case signal_number::kSigIll:
        return "SIGILL";
      case signal_number::kSigAbrt:
        return "SIGABRT";
      case signal_number::kSigFpe:
        return "SIGFPE";
      case signal_number::kSigKill:
        return "SIGKILL";
      case signal_number::kSigSegv:
        return "SIGSEGV";
      case signal_number::kSigPipe:
        return "SIGPIPE";
      case signal_number::kSigAlrm:
        return "SIGALRM";
      case signal_number::kSigTerm:
        return "SIGTERM";
      case signal_number::kSigChld:
        return "SIGCHLD";
      case signal_number::kSigCont:
        return "SIGCONT";
      case signal_number::kSigStop:
        return "SIGSTOP";
      case signal_number::kSigTstp:
        return "SIGTSTP";
      case signal_number::kSigUsr1:
        return "SIGUSR1";
      case signal_number::kSigUsr2:
        return "SIGUSR2";
      default:
        return "SIG???";
    }
  }

  /// @name 构造/析构函数
  /// @{
  SignalState() = default;
  SignalState(const SignalState&) = delete;
  SignalState(SignalState&&) = delete;
  auto operator=(const SignalState&) -> SignalState& = delete;
  auto operator=(SignalState&&) -> SignalState& = delete;
  ~SignalState() = default;
  /// @}
};

/**
 * @brief 获取信号的默认动作
 * @param signum 信号编号
 * @return 默认动作 ('T'=终止, 'I'=忽略, 'C'=核心转储, 'S'=停止, 'K'=继续)
 */
[[nodiscard]] constexpr auto GetDefaultSignalAction(int signum) -> char {
  switch (signum) {
    case signal_number::kSigChld:
      // 忽略
      return 'I';
    case signal_number::kSigCont:
      // 继续
      return 'K';
    case signal_number::kSigStop:
    case signal_number::kSigTstp:
      // 停止
      return 'S';
    case signal_number::kSigQuit:
    case signal_number::kSigIll:
    case signal_number::kSigAbrt:
    case signal_number::kSigFpe:
    case signal_number::kSigSegv:
      // 核心转储 (简化为终止)
      return 'C';
    default:
      // 终止
      return 'T';
  }
}
