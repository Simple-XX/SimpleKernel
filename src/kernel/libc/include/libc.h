
/**
 * @file libc.h
 * @brief libc 头文件
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

#ifndef SIMPLEKERNEL_SRC_KERNEL_LIBC_INCLUDE_LIBC_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 获取调用栈
 * @param buffer 指向一个数组，该数组用于存储调用栈中的返回地址
 * @param size 数组的大小，即调用栈中最多存储多少个返回地址
 * @return int 成功时返回实际写入数组中的地址数量，失败时返回 -1
 */
int backtrace(void **buffer, int size);

/**
 * 获取调用栈可读符号
 * @param buffer 指向一个存有调用栈返回地址的数组，保存输出
 * @param size 数组中的地址数量，即调用栈的深度
 * @return char**
 * 返回一个指向字符串数组的指针，每个字符串描述一个调用栈帧。失败时返回 NULL
 */
char **backtrace_symbols(void *const *buffer, int size);

#ifdef __cplusplus
}
#endif

#define SIMPLEKERNEL_SRC_KERNEL_LIBC_INCLUDE_LIBC_H_

#endif /* SIMPLEKERNEL_SRC_KERNEL_LIBC_INCLUDE_LIBC_H_ */
