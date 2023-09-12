
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

#include "ostream.hpp"

[[maybe_unused]] auto wait_for_input(EFI_INPUT_KEY *_key) -> EFI_STATUS {
  EFI_STATUS status = EFI_SUCCESS;
  do {
    status = uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2, ST->ConIn, _key);
  } while (EFI_NOT_READY == status);

  return status;
}

auto ostream::operator<<(ostream &(*_ostream)(ostream &)) -> ostream & {
  return _ostream(*this);
}

auto ostream::hex_x(ostream &_ostream) -> ostream & {
  _ostream.mode = ostream::x;
  return _ostream;
}

auto ostream::hex_X(ostream &_ostream) -> ostream & {
  _ostream.mode = ostream::X;
  return _ostream;
}

auto ostream::endl(ostream &_ostream) -> ostream & { return _ostream << L'\n'; }
