
/**
 * @file drv.cpp
 * @brief 设备驱动基类实现
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-12-01<td>MRNIU<td>迁移到 doxygen
 * </table>
 */

#include "drv.h"

drv_t::drv_t(void) {
    name = "drv_t";
    return;
}

drv_t::~drv_t(void) {
    return;
}
