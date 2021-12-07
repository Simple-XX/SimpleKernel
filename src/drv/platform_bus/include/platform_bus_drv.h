
/**
 * @file platform_bus_drv.h
 * @brief 平台总线驱动头文件
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

#ifndef _PLATFORM_BUS_DRV_H_
#define _PLATFORM_BUS_DRV_H_

#include "drv.h"

// 平台总线
struct platform_bus_drv_t : drv_t {
private:
protected:
public:
    platform_bus_drv_t(void);
    platform_bus_drv_t(const resource_t &_resource);
    ~platform_bus_drv_t(void);
    bool init(void) override final;
};

declare_call_back(platform_bus_drv_t);

#endif /* _PLATFORM_BUS_DRV_H_ */
