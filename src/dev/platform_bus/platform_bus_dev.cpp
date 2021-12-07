
/**
 * @file platform_bus_dev.cpp
 * @brief 平台总线基类实现
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

#include "platform_bus_dev.h"
#include "platform_bus_drv.h"

platform_bus_dev_t::platform_bus_dev_t(void) {
    compatible_name = "platform_bus driver";
    return;
}

platform_bus_dev_t::platform_bus_dev_t(const resource_t &_resource)
    : bus_dev_t(_resource) {
    // platform 总线不需要驱动
    compatible_name = "platform_bus driver";
    drv             = new platform_bus_drv_t();
    return;
}

platform_bus_dev_t::~platform_bus_dev_t(void) {
    if (drv != nullptr) {
        delete drv;
    }
    return;
}
