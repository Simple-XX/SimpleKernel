
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// platform_bus_drv.h for Simple-XX/SimpleKernel.

#ifndef _PLATFORM_BUS_DRV_H_
#define _PLATFORM_BUS_DRV_H_

#include "drv.h"

// 平台总线
struct platform_bus_drv_t : drv_t {
private:
protected:
public:
    platform_bus_drv_t(void);
    ~platform_bus_drv_t(void);
    bool init(void) override final;
};

#endif /* _PLATFORM_BUS_DRV_H_ */
