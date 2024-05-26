
/**
 * @file ostream.hpp
 * @brief ostream hpp
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

#ifndef SIMPLEKERNEL_SRC_BOOT_INCLUDE_OUT_STREAM_HPP_
#define SIMPLEKERNEL_SRC_BOOT_INCLUDE_OUT_STREAM_HPP_

#include <cstdio>

#ifdef __cplusplus
extern "C" {
#endif

#include "efi.h"
#include "efilib.h"

#ifdef __cplusplus
}
#endif

/**
 * 等待输入
 * @param _key 输入的按键
 * @return efi 错误码
 */
[[maybe_unused]] auto wait_for_input(EFI_INPUT_KEY *_key) -> EFI_STATUS;

/**
 * 输出流
 */
class OutStream {
 public:
  /**
   * 使用默认构造
   */
  OutStream() = default;

  /**
   * 使用默认析构
   */
  ~OutStream() = default;

  /// @name 不使用的构造函数
  /// @{
  OutStream(const OutStream &) = delete;
  OutStream(OutStream &&) = delete;
  auto operator=(const OutStream &) -> OutStream & = delete;
  auto operator=(OutStream &&) -> OutStream & = delete;
  /// @}

  /**
   * 输出类型
   * @tparam _t 模板类型
   * @param val 数据
   * @return 输出流
   */
  template <class _t>
  inline auto operator<<(_t val) -> OutStream &;

  /**
   * ‘<<’ 操作符重载
   * @param ostream 输出流
   * @return 输出流
   */
  auto operator<<(OutStream &(*ostream)(OutStream &)) -> OutStream &;

  /**
   * 输出 16 进制
   * @param ostream 输出流
   * @return 输出流
   */
  static auto hex_x(OutStream &ostream) -> OutStream &;

  /**
   * 输出 16 进制
   * @param ostream 输出流
   * @return 输出流
   */
  static auto hex_X(OutStream &ostream) -> OutStream &;

  /**
   * 输出换行
   * @param ostream
   * @return 输出流
   */
  static auto endl(OutStream &ostream) -> OutStream &;

 private:
  /**
   * 输出流模式枚举
   */
  enum Mode : uint8_t {
    /// 16 进制 %x
    x,
    /// 16 进制 %X
    X,
    /// 10 进制 %d
    d,
  };
  /// 输出流模式
  Mode mode_;
};

/// @name 输出流特化
/// @{

template <class _t>
auto OutStream::operator<<(_t val) -> OutStream & {
  *this << (uint64_t)val;
  return *this;
}

template <>
inline auto OutStream::operator<<(int32_t val) -> OutStream & {
  if (mode_ == d) {
    Print(L"%d", val);
  } else if (mode_ == x) {
    Print(L"0x%x", val);
  } else if (mode_ == X) {
    Print(L"0x%X", val);
  }
  mode_ = d;
  return *this;
}

template <>
inline auto OutStream::operator<<(uint32_t val) -> OutStream & {
  if (mode_ == d) {
    Print(L"%d", val);
  } else if (mode_ == x) {
    Print(L"0x%x", val);
  } else if (mode_ == X) {
    Print(L"0x%X", val);
  }
  mode_ = d;
  return *this;
}

template <>
inline auto OutStream::operator<<(int64_t val) -> OutStream & {
  if (mode_ == d) {
    Print(L"%ld", val);
  } else if (mode_ == x) {
    Print(L"0x%x", val);
  } else if (mode_ == X) {
    Print(L"0x%X", val);
  }
  mode_ = d;
  return *this;
}

template <>
inline auto OutStream::operator<<(uint64_t val) -> OutStream & {
  if (mode_ == d) {
    Print(L"%ld", val);
  } else if (mode_ == x) {
    Print(L"0x%x", val);
  } else if (mode_ == X) {
    Print(L"0x%X", val);
  }
  mode_ = d;
  return *this;
}

template <>
inline auto OutStream::operator<<(wchar_t val) -> OutStream & {
  Print(L"%c", val);
  mode_ = d;
  return *this;
}

template <>
inline auto OutStream::operator<<(const wchar_t *val) -> OutStream & {
  Print(L"%s", val);
  mode_ = d;
  return *this;
}

template <>
inline auto OutStream::operator<<(void *val) -> OutStream & {
  *this << reinterpret_cast<uint64_t>(val);
  return *this;
}

template <>
inline auto OutStream::operator<<(const void *val) -> OutStream & {
  *this << reinterpret_cast<uint64_t>(val);
  return *this;
}

/// @}

/// 全局输出流
extern OutStream debug;

#endif /* SIMPLEKERNEL_SRC_BOOT_INCLUDE_OUT_STREAM_HPP_ */
