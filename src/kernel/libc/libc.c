
/**
 * @file libc.cpp
 * @brief libc cpp
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

#include "libc.h"

/// 由于使用了
/// stdlib，其中部分依赖的符号需要手动定义，实际上并不会被使用，如果被使用应当出错
// __libc_start_main_impl 依赖
void __attribute__((weak)) _init() { ; }
// call_fini 依赖
void __attribute__((weak)) _fini() { ; }
// _dl_relocate_static_pie 依赖
void __attribute__((weak)) _DYNAMIC() { ; }
