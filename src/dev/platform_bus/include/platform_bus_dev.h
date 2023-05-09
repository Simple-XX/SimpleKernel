
/**
 * @file platform_bus_dev.h
 * @brief 平台总线基类头文件
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

#ifndef SIMPLEKERNEL_PLATFORM_BUS_DEV_H
#define SIMPLEKERNEL_PLATFORM_BUS_DEV_H

#include "bus_dev.h"
#include "dev.h"
#include "resource.h"
#include "string"
#include "vector"
#include "virtio_dev.h"

// virtio bus 设备
// 这是一个虚拟总线
class platform_bus_dev_t : public bus_dev_t {
private:

protected:

public:
    platform_bus_dev_t(void);
    platform_bus_dev_t(const resource_t& _resource);
    ~platform_bus_dev_t(void);
};

#endif /* SIMPLEKERNEL_PLATFORM_BUS_DEV_H */
