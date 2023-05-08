
/**
 * @file stack_chk.cpp
 * @brief 栈保护
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-05-08
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * Based on https://github.com/MRNIU/MiniCRT
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-05-08<td>Zone.N<td>创建文件
 * </table>
 */

#include "cstdint"
#include "cstdio"

#if UINT32_MAX == UINTPTR_MAX
#    define STACK_CHK_GUARD 0xe2dee396
#else
#    define STACK_CHK_GUARD 0x595e9fbd94fda766
#endif

uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void      __stack_chk_fail(void) {
    err("Stack smashing detected\n");
    return;
}
