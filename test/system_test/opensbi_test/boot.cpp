
/**
 * @file boot.cpp
 * @brief boot cpp
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

#ifdef __cplusplus
extern "C" {
#endif

#include "cstdint"

#include "sbi/sbi_ecall_interface.h"

struct sbiret_t {
  /// 错误码
  long error;
  /// 返回值
  long value;
};

sbiret_t ecall(unsigned long _arg0, unsigned long _arg1, unsigned long _arg2,
               unsigned long _arg3, unsigned long _arg4, unsigned long _arg5,
               int _fid, int _eid) {
  sbiret_t ret;
  register uintptr_t a0 asm("a0") = (uintptr_t)(_arg0);
  register uintptr_t a1 asm("a1") = (uintptr_t)(_arg1);
  register uintptr_t a2 asm("a2") = (uintptr_t)(_arg2);
  register uintptr_t a3 asm("a3") = (uintptr_t)(_arg3);
  register uintptr_t a4 asm("a4") = (uintptr_t)(_arg4);
  register uintptr_t a5 asm("a5") = (uintptr_t)(_arg5);
  register uintptr_t a6 asm("a6") = (uintptr_t)(_fid);
  register uintptr_t a7 asm("a7") = (uintptr_t)(_eid);
  asm("ecall"
      : "+r"(a0), "+r"(a1)
      : "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7)
      : "memory");
  ret.error = a0;
  ret.value = a1;
  return ret;
}

void put_char(const char _c) {
  ecall(_c, 0, 0, 0, 0, 0, 0, SBI_EXT_0_1_CONSOLE_PUTCHAR);
  return;
}

int main(int, char **) {
  put_char('H');
  put_char('e');
  put_char('l');
  put_char('l');
  put_char('W');
  put_char('o');
  put_char('r');
  put_char('l');
  put_char('d');
  put_char('!');

  return 0;
}

#ifdef __cplusplus
}
#endif
