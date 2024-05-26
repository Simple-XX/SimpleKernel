
/**
 * @file ostream.cpp
 * @brief 输出
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

#include "out_stream.hpp"

[[maybe_unused]] auto wait_for_input(EFI_INPUT_KEY *_key) -> EFI_STATUS {
  EFI_STATUS status = EFI_SUCCESS;
  do {
    status = uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2, ST->ConIn, _key);
  } while (EFI_NOT_READY == status);

  return status;
}

auto OutStream::operator<<(OutStream &(*_ostream)(OutStream &)) -> OutStream & {
  return _ostream(*this);
}

auto OutStream::hex_x(OutStream &_ostream) -> OutStream & {
  _ostream.mode_ = OutStream::x;
  return _ostream;
}

auto OutStream::hex_X(OutStream &_ostream) -> OutStream & {
  _ostream.mode_ = OutStream::X;
  return _ostream;
}

auto OutStream::endl(OutStream &_ostream) -> OutStream & {
  return _ostream << L'\n';
}

/// 全局输出流
OutStream debug;
