
/**
 * @file kernel_log.hpp
 * @brief 内核日志相关函数
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-07-15
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-07-15<td>Zone.N (Zone.Niuzh@hotmail.com)<td>创建文件
 * </table>
 */

#ifndef SIMPLEKERNEL_SRC_KERNEL_INCLUDE_KERNEL_LOG_HPP_
#define SIMPLEKERNEL_SRC_KERNEL_INCLUDE_KERNEL_LOG_HPP_

#include <stdarg.h>

#include "cstdio"

namespace {

/// ANSI 转义码，在支持 ANSI 转义码的终端中可以显示颜色
static constexpr const auto kReset = "\033[0m";
static constexpr const auto kRed = "\033[31m";
static constexpr const auto kGreen = "\033[32m";
static constexpr const auto kYellow = "\033[33m";
static constexpr const auto kBlue = "\033[34m";
static constexpr const auto kMagenta = "\033[35m";
static constexpr const auto kCyan = "\033[36m";
static constexpr const auto kWhite = "\033[37m";

}  // namespace

/**
 * @brief 与 printf 类似，只是颜色不同
 */
extern "C" inline void info(const char* format, ...) {
  va_list args;
  va_start(args, format);
  printf("%s", kCyan);
  vprintf(format, args);
  printf("%s", kReset);
  va_end(args);
}

extern "C" inline void warn(const char* format, ...) {
  va_list args;
  va_start(args, format);
  printf("%s", kYellow);
  vprintf(format, args);
  printf("%s", kReset);
  va_end(args);
}

extern "C" inline void err(const char* format, ...) {
  va_list args;
  va_start(args, format);
  printf("%s", kRed);
  vprintf(format, args);
  printf("%s", kReset);
  va_end(args);
}

#endif /* SIMPLEKERNEL_SRC_KERNEL_INCLUDE_KERNEL_LOG_HPP_ */
