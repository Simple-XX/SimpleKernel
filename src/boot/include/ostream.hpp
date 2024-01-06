
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

#ifndef SIMPLEKERNEL_OSTREAM_HPP
#define SIMPLEKERNEL_OSTREAM_HPP

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
class ostream {
 public:
  /**
   * 使用默认构造
   */
  ostream() = default;

  /**
   * 使用默认析构
   */
  ~ostream() = default;

  /// @name 不使用的构造函数
  /// @{
  ostream(const ostream &) = delete;
  ostream(ostream &&) = delete;
  auto operator=(const ostream &) -> ostream & = delete;
  auto operator=(ostream &&) -> ostream & = delete;
  /// @}

  /**
   * 输出类型
   * @tparam _t 模板类型
   * @param _val 数据
   * @return 输出流
   */
  template <class _t>
  inline auto operator<<(_t _val) -> ostream &;

  /**
   * ‘<<’ 操作符重载
   * @param _ostream 输出流
   * @return 输出流
   */
  auto operator<<(ostream &(*_ostream)(ostream &)) -> ostream &;

  /**
   * 输出 16 进制
   * @param _ostream 输出流
   * @return 输出流
   */
  static auto hex_x(ostream &_ostream) -> ostream &;

  /**
   * 输出 16 进制
   * @param _ostream 输出流
   * @return 输出流
   */
  static auto hex_X(ostream &_ostream) -> ostream &;

  /**
   * 输出换行
   * @param _ostream
   * @return 输出流
   */
  static auto endl(ostream &_ostream) -> ostream &;

 private:
  /**
   * 输出流模式枚举
   */
  enum mode_t : uint8_t {
    /// 16 进制 %x
    x,
    /// 16 进制 %X
    X,
    /// 10 进制 %d
    d,
  };
  /// 输出流模式
  mode_t mode;
};

/// @name 输出流特化
/// @{

template <class _t>
auto ostream::operator<<(_t _val) -> ostream & {
  *this << (uint64_t)_val;
  return *this;
}

template <>
inline auto ostream::operator<<(int32_t _val) -> ostream & {
  if (mode == d) {
    Print(L"%d", _val);
  } else if (mode == x) {
    Print(L"0x%x", _val);
  } else if (mode == X) {
    Print(L"0x%X", _val);
  }
  mode = d;
  return *this;
}

template <>
inline auto ostream::operator<<(uint32_t _val) -> ostream & {
  if (mode == d) {
    Print(L"%d", _val);
  } else if (mode == x) {
    Print(L"0x%x", _val);
  } else if (mode == X) {
    Print(L"0x%X", _val);
  }
  mode = d;
  return *this;
}

template <>
inline auto ostream::operator<<(int64_t _val) -> ostream & {
  if (mode == d) {
    Print(L"%ld", _val);
  } else if (mode == x) {
    Print(L"0x%x", _val);
  } else if (mode == X) {
    Print(L"0x%X", _val);
  }
  mode = d;
  return *this;
}

template <>
inline auto ostream::operator<<(uint64_t _val) -> ostream & {
  if (mode == d) {
    Print(L"%ld", _val);
  } else if (mode == x) {
    Print(L"0x%x", _val);
  } else if (mode == X) {
    Print(L"0x%X", _val);
  }
  mode = d;
  return *this;
}

template <>
inline auto ostream::operator<<(wchar_t _val) -> ostream & {
  Print(L"%c", _val);
  mode = d;
  return *this;
}

template <>
inline auto ostream::operator<<(const wchar_t *_val) -> ostream & {
  Print(L"%s", _val);
  mode = d;
  return *this;
}

template <>
inline auto ostream::operator<<(void *_val) -> ostream & {
  *this << reinterpret_cast<uint64_t>(_val);
  return *this;
}

template <>
inline auto ostream::operator<<(const void *_val) -> ostream & {
  *this << reinterpret_cast<uint64_t>(_val);
  return *this;
}

/// @}

/// 全局输出流
static ostream debug;

#endif /* SIMPLEKERNEL_OSTREAM_HPP */
