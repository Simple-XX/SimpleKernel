
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// dev_drv_manager.h for Simple-XX/SimpleKernel.

#ifndef _DEV_DRV_MANAGER_H_
#define _DEV_DRV_MANAGER_H_

#include "stdint.h"
#include "vector"
#include "string"
#include "dev.h"
#include "drv.h"
#include "bus.h"

// 总线 设备与内核的通信方式
// 设备 挂载在总线上的硬件，总线是挂载在 null 上的一种设备
// 驱动 驱动总线/设备的代码
// 设备与驱动管理
class DEV_DRV_MANAGER {
private:
    // 总线列表
    mystl::vector<bus_t *> buss;
    // 设备列表
    mystl::vector<dev_t *> devs;
    // 驱动列表
    mystl::vector<drv_t *> drvs;
    // 匹配驱动
    bool match(dev_t &_dev, drv_t &_drv);
    // 输出设备列表
    void show(void) const;

protected:
public:
    DEV_DRV_MANAGER(void);
    ~DEV_DRV_MANAGER(void);
    // 添加总线
    bool add_bus(bus_t &_bus);
    // 添加驱动
    bool add_drv(drv_t &_drv);
    // 添加设备
    bool add_dev(dev_t &_dev);
    // 初始化指定设备
    bool init(dev_t &_dev);
};

#endif /* _DEV_DRV_MANAGER_H_ */
