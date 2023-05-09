
/**
 * @file platform_bus_drv.cpp
 * @brief 平台总线驱动实现
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-12-07<td>MRNIU<td>迁移到 doxygen
 * </table>
 */

#include "platform_bus_drv.h"

platform_bus_drv_t::platform_bus_drv_t(void)
    : drv_t("platform_bus driver", "platform_bus_drv_t") {
    return;
}

platform_bus_drv_t::platform_bus_drv_t(const resource_t&)
    : drv_t("platform_bus driver", "platform_bus_drv_t") {
    return;
}

platform_bus_drv_t::~platform_bus_drv_t(void) {
    return;
}

// platform 总线不需要初始化
bool platform_bus_drv_t::init(void) {
    info("platform_bus driver init successful.\n");
    return true;
}

define_call_back(platform_bus_drv_t);
