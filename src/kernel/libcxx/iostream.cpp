
/**
 * @file iostream
 * @brief C++ 输入输出
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * Based on https://github.com/MRNIU/MiniCRT
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-09-18<td>digmouse233<td>迁移到 doxygen
 * </table>
 */

#include "iostream"

#include <cstdint>

#include "cstdio"

namespace std {

ostream& ostream::operator<<(int8_t val) {
  printf("%d", val);
  return *this;
}

ostream& ostream::operator<<(uint8_t val) {
  printf("%d", val);
  return *this;
}

ostream& ostream::operator<<(const char* val) {
  printf("%s", val);
  return *this;
}

ostream& ostream::operator<<(int16_t val) {
  printf("%d", val);
  return *this;
}

ostream& ostream::operator<<(uint16_t val) {
  printf("%d", val);
  return *this;
}

ostream& ostream::operator<<(int32_t val) {
  printf("%d", val);
  return *this;
}

ostream& ostream::operator<<(uint32_t val) {
  printf("%d", val);
  return *this;
}

ostream& ostream::operator<<(int64_t val) {
  printf("%ld", val);
  return *this;
}

ostream& ostream::operator<<(uint64_t val) {
  printf("%ld", val);
  return *this;
}

ostream& ostream::operator<<(ostream& (*manip)(ostream&)) {
  return manip(*this);
}

};  // namespace std
