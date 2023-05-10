
/**
 * @file buf.h
 * @brief 设备缓冲区头文件
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-05-10
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-05-10<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef SIMPLEKERNEL_BUF_H
#define SIMPLEKERNEL_BUF_H

#include "common.h"
#include "cstdint"

/**
 * @brief 用于设备与内存数据交换
 */
class buf_t {
private:

protected:

public:
    /// 是否已经取回数据
    bool     valid;
    /// 硬盘扇区号
    uint64_t sector;
    /// 数据缓冲
    uint8_t  data[COMMON::BUFFFER_SIZE];

    buf_t(void)  = default;
    ~buf_t(void) = default;
};

#endif /* SIMPLEKERNEL_BUF_H */
