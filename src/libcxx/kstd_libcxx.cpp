/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <cpu_io.h>
#include <stdint.h>

#include <algorithm>
#include <atomic>

#include "arch.h"
#include "kernel_log.hpp"

/// 全局构造函数函数指针
using function_t = void (*)();
// 在 link.ld 中定义
/// 全局构造函数函数指针起点地址
extern "C" function_t __init_array_start;
/// 全局构造函数函数指针终点地址
extern "C" function_t __init_array_end;
/// 全局析构函数函数指针起点地址
extern "C" function_t __fini_array_start;
/// 全局析构函数函数指针终点地址
extern "C" function_t __fini_array_end;
/// 动态共享对象标识，内核使用静态链接，此变量在内核中没有使用
void* __dso_handle = nullptr;

/// 最大析构函数数量
static constexpr size_t kMaxAtExitFuncsCount = 128;

/// 析构函数结构
struct atexit_func_entry_t {
  // 析构函数指针
  void (*destructor_func)(void*);
  // 要析构的对象
  void* obj_ptr;
  // 共享库对象，在内核中没有使用
  void* dso_handle;
};

/// 析构函数数组
static atexit_func_entry_t atexit_funcs[kMaxAtExitFuncsCount];  // NOLINT
/// 析构函数个数
static size_t atexit_func_count = 0;

/**
 * 注册在程序正常终止时调用的析构函数
 * @param destructor_func 指向要调用的析构函数的指针
 * @param obj_ptr 传递给析构函数的参数
 * @return 成功返回 0
 */
extern "C" auto __cxa_atexit(void (*destructor_func)(void*), void* obj_ptr,
                             void*) -> int {
  if (atexit_func_count >= kMaxAtExitFuncsCount) {
    return -1;
  }
  atexit_funcs[atexit_func_count].destructor_func = destructor_func;
  atexit_funcs[atexit_func_count].obj_ptr = obj_ptr;
  atexit_funcs[atexit_func_count].dso_handle = nullptr;
  atexit_func_count++;
  return 0;
}

/**
 * 调用析构函数
 * @param destructor_func 要调用的析构函数指针，为 nullptr
 * 时调用所有注册的析构函数。
 */
extern "C" auto __cxa_finalize(void* destructor_func) -> void {
  if (destructor_func == nullptr) {
    // 如果 destructor_func 为 nullptr，调用所有析构函数
    for (size_t i = atexit_func_count; i > 0; --i) {
      size_t idx = i - 1;
      if (atexit_funcs[idx].destructor_func != nullptr) {
        (*atexit_funcs[idx].destructor_func)(atexit_funcs[idx].obj_ptr);
      }
    }
  } else {
    // 不为空时只调用对应的析构函数
    for (size_t i = atexit_func_count; i > 0; --i) {
      size_t idx = i - 1;
      if (reinterpret_cast<void*>(atexit_funcs[idx].destructor_func) ==
          destructor_func) {
        (*atexit_funcs[idx].destructor_func)(atexit_funcs[idx].obj_ptr);
        atexit_funcs[idx].destructor_func = nullptr;
      }
    }
  }
}

/// @name 保证静态局部变量线程安全
/// @todo 确保正确
/// @{
/**
 * if ( obj_guard.first_byte == 0 ) {
 *     if ( __cxa_guard_acquire(&obj_guard) ) {
 *       try {
 *         ... initialize the object ...;
 *       }
 *       catch (...) {
 *         __cxa_guard_abort(&obj_guard);
 *         throw;
 *       }
 *       ... queue object destructor with __cxa_atexit() ...;
 *       __cxa_guard_release(&obj_guard);
 *     }
 *  }
 */

/// @note 根据 Itanium C++ ABI，守护变量必须是 64 位
/// @see https://itanium-cxx-abi.github.io/cxx-abi/abi.html#once-ctor
struct GuardType {
  /// 原子守护变量：bit 0 = is_initialized, bit 8 = is_in_use
  std::atomic<uint64_t> guard{0};

  static constexpr uint64_t kInitializedMask = 0x01;
  static constexpr uint64_t kInUseMask = 0x100;
};

static_assert(sizeof(GuardType) == 8, "GuardType must be 64 bits per ABI");

/**
 * 检测静态局部变量是否已经初始化
 * @param guard 锁，一个 64 位变量
 * @return 未初始化返回非零值，已初始化返回 0
 * @note 使用紧凑的 CAS 循环，compare_exchange_weak 失败时会自动更新 expected
 */
extern "C" auto __cxa_guard_acquire(GuardType* guard) -> int {
  uint64_t expected = 0;

  // 紧凑的 CAS 循环：尝试将 0 -> kInUseMask
  // compare_exchange_weak 失败时会将当前值写入 expected
  while (!guard->guard.compare_exchange_weak(expected, GuardType::kInUseMask,
                                             std::memory_order_acq_rel,
                                             std::memory_order_acquire)) {
    // CAS 失败，检查失败原因
    if ((expected & GuardType::kInitializedMask) != 0U) {
      // 已被其他核心初始化完成
      return 0;
    }

    // 其他核心正在初始化中，等待
    cpu_io::Pause();

    // 重置 expected 以便下次 CAS 尝试
    // 如果当前值仍有 kInUseMask，CAS 会再次失败并更新 expected
    expected = 0;
  }

  // CAS 成功，当前核心负责初始化
  return 1;
}

/**
 * 用于检测静态局部变量是否已经初始化，并设置锁
 * @param guard 锁，一个 64 位变量
 * @return 未初始化返回非零值并设置锁，已初始化返回 0
 */
extern "C" auto __cxa_guard_release(GuardType* guard) -> void {
  // 设置 initialized 位，清除 in_use 位
  guard->guard.store(GuardType::kInitializedMask, std::memory_order_release);
}

/**
 * 如果在初始化过程中出现异常或其他错误，调用此函数以释放锁而不标记变量为已初始化
 * @param guard 锁
 */
extern "C" auto __cxa_guard_abort(GuardType* guard) -> void {
  // 清除所有位
  guard->guard.store(0, std::memory_order_release);
}

/// @}

/**
 * 纯虚函数调用处理
 */
extern "C" auto __cxa_pure_virtual() -> void {
  while (true) {
    cpu_io::Pause();
  }
}

/**
 * 终止程序
 */
extern "C" auto abort() -> void {
  while (true) {
    cpu_io::Pause();
  }
}

/**
 * @brief libc assert() 失败时调用的底层接口
 * @param assertion  断言表达式字符串
 * @param file       源文件名
 * @param line       行号
 * @param function   函数名
 */
extern "C" [[noreturn]] void __assert_fail(const char* assertion,
                                           const char* file, unsigned int line,
                                           const char* function) {
  klog::RawPut("\n[ASSERT FAILED] ");
  klog::RawPut(file);
  etl_putchar(':');
  char buf_line[12];
  auto* end = etl::format_to_n(buf_line, sizeof(buf_line) - 1, "{}", line);
  *end = '\0';
  klog::RawPut(" in ");
  klog::RawPut(function);
  klog::RawPut("\n Expression: ");
  klog::RawPut(assertion);
  etl_putchar('\n');
  RawDumpStack();
  while (1) {
    cpu_io::Pause();
  }
}

/**
 * c++ 全局对象构造
 */
auto CppInit() -> void {
  // 调用构造函数
  std::for_each(&__init_array_start, &__init_array_end,
                [](function_t func) { (func)(); });
}

/**
 * c++ 全局对象析构
 */
auto CppDeInit() -> void {
  // 调用析构函数
  std::for_each(&__fini_array_start, &__fini_array_end,
                [](function_t func) { (func)(); });
}
