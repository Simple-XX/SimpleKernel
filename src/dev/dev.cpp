
/**
 * @file dev.cpp
 * @brief 设备基类
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

#include "dev.h"

dev_t::dev_t(void) {
    dev_name = "dev_t";
    drv_name = "";
    bus_name = "";
    drv      = nullptr;
// #define DEBUG
#ifdef DEBUG
    printf("dev_t ctor.\n");
#undef DEBUG
#endif
    return;
}

dev_t::dev_t(const resource_t &_resource) : resource(_resource) {
    dev_name = resource.name;
    drv_name = "";
    bus_name = "";
    drv      = nullptr;
// #define DEBUG
#ifdef DEBUG
    printf("dev_t ctor.\n");
#undef DEBUG
#endif
    return;
}

dev_t::~dev_t(void) {
    return;
}
