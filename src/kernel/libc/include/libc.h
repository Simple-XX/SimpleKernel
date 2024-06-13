
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

#ifdef __cplusplus
}
#endif

#define SIMPLEKERNEL_SRC_KERNEL_LIBC_INCLUDE_LIBC_H_

#endif /* SIMPLEKERNEL_SRC_KERNEL_LIBC_INCLUDE_LIBC_H_ */
