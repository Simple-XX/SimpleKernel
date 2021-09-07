
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// platform_bus_drv.h for Simple-XX/SimpleKernel.

#ifndef _PLATFORM_BUS_DRV_H_
#define _PLATFORM_BUS_DRV_H_

#include "vector"
#include "dev.h"
#include "drv.h"
#include "resource.h"

// 平台总线
struct platform_bus_drv_t : drv_t {
private:
    // 总线上的设备
    mystl::vector<dev_t *> devs;
    // 设备对应的资源
    mystl::vector<resource_t *> res;

protected:
public:
    // TODO: 填充内容
    platform_bus_drv_t(void);
    ~platform_bus_drv_t(void);
    virtual bool init(void);
    virtual void read(void);
    virtual void write(void);
};

#endif /* _PLATFORM_BUS_DRV_H_ */
