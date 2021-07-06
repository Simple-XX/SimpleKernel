
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

// 设备与驱动管理
class DEV_DRV_MANAGER {
private:
    // 设备列表
    mystl::vector<dev_t *> devs;
    // 驱动列表
    mystl::vector<drv_t *> drvs;
    // 匹配驱动
    bool match(dev_t &_dev, drv_t &_drv);

protected:
public:
    DEV_DRV_MANAGER(void);
    ~DEV_DRV_MANAGER(void);
    // 添加驱动
    bool add_drv(drv_t &_drv);
    // 添加设备
    bool add_dev(dev_t &_dev);
    // 初始化指定设备
    bool init(dev_t &_dev);
};

#endif /* _DEV_DRV_MANAGER_H_ */
