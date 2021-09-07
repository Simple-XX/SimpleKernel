
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// bus.h for Simple-XX/SimpleKernel.

#ifndef _BUS_H_
#define _BUS_H_

#include "stdint.h"
#include "vector"
#include "string"
#include "dev.h"
#include "resource.h"

// 总线设备抽象
// 总线设备是用于内核与设备通信的设备
struct bus_t : dev_t {
private:
    // 设备向量
    mystl::vector<const dev_t *> devs;

protected:
public:
    // 用于创建虚拟总线
    bus_t(void);
    bus_t(const resource_t &_resource);
    virtual ~bus_t(void) = 0;
    friend std::ostream &operator<<(std::ostream &_out, bus_t &_bus) {
        info("dev_name: %s, drv_name: %s, bus_name: %s, drv: 0x%p",
             _bus.dev_name.c_str(), _bus.drv_name.c_str(),
             _bus.bus_name.c_str(), _bus.drv);
        return _out;
    }
};

#endif /* _BUS_H_ */
