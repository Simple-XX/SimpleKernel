
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
#include "bus_dev.h"

// 总线 设备与内核的通信方式
// 设备 挂载在总线上的硬件，总线是挂载在 null 上的一种设备
// 驱动 驱动总线/设备的代码
// 设备与驱动管理
class DEV_DRV_MANAGER {
private:
    /// @todo 总线驱动管理
    // 总线列表
    mystl::vector<bus_dev_t *> buss;
    // 输出设备列表
    void show(void) const;
    // 初始化所有总线
    bool buss_init(void);

protected:
public:
    DEV_DRV_MANAGER(void);
    ~DEV_DRV_MANAGER(void);
    // 添加总线
    bool add_bus(bus_dev_t &_bus);
};

#endif /* _DEV_DRV_MANAGER_H_ */
