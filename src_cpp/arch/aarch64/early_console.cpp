/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "pl011/pl011_driver.hpp"
#include "pl011_singleton.h"

namespace {

pl011::Pl011Device* pl011_uart = nullptr;

/**
 * @brief 早期控制台初始化类
 * @note 通过全局构造在内核 main 之前初始化 PL011 串口输出
 */
class EarlyConsole {
 public:
  /// @name 构造/析构函数
  /// @{

  /// 初始化 PL011 串口输出
  EarlyConsole() {
    Pl011Singleton::create(SIMPLEKERNEL_EARLY_CONSOLE_BASE);
    pl011_uart = &Pl011Singleton::instance();
  }

  EarlyConsole(const EarlyConsole&) = delete;
  EarlyConsole(EarlyConsole&&) = delete;
  auto operator=(const EarlyConsole&) -> EarlyConsole& = delete;
  auto operator=(EarlyConsole&&) -> EarlyConsole& = delete;
  ~EarlyConsole() = default;
  /// @}
};

EarlyConsole early_console;

}  // namespace

/**
 * @brief 早期控制台字符输出
 * @param c 要输出的字符
 */
extern "C" auto etl_putchar(int c) -> void {
  if (pl011_uart) {
    pl011_uart->PutChar(c);
  }
}
