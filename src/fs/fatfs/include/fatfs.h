
/**
 * @file fatfs.h
 * @brief fat 文件系统头文件
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-05-08
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-05-08<td>Zone.N<td>创建文件
 * </table>
 */

#ifndef SIMPLEKERNEL_FATFS_H
#define SIMPLEKERNEL_FATFS_H

#include "ff.h"

class fatfs_t {
private:

public:
    fatfs_t(void);
    ~fatfs_t(void);
};

#endif /* SIMPLEKERNEL_FATFS_H */
