
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// bus_dev.h for Simple-XX/SimpleKernel.

#ifndef _BUS_DEV_H_
#define _BUS_DEV_H_

#include "stdint.h"
#include "vector"
#include "string"
#include "dev.h"
#include "resource.h"
#include "cxxabi.h"

// 总线设备抽象
// 总线设备是用于内核与设备通信的设备
struct bus_dev_t : dev_t {
private:
protected:
public:
    // 设备列表
    mystl::vector<dev_t *> devs;
    // 驱动列表
    mystl::vector<drv_t *>                                   drvs;
    mystl::vector<mystl::pair<mystl::string, mystl::string>> drvs_name;
    // 用于创建虚拟总线
    bus_dev_t(void);
    bus_dev_t(const resource_t &_resource);
    virtual ~bus_dev_t(void) = 0;
    // 添加驱动
    bool add_drv(const mystl::string &_drv_name,
                 const mystl::string &_type_name);
    // 添加设备
    bool add_dev(dev_t *_dev);
    // 匹配设备与驱动
    virtual bool         match(dev_t                                     &_dev,
                               mystl::pair<mystl::string, mystl::string> &_name_pair);
    void                 show(void) const;
    friend std::ostream &operator<<(std::ostream &_out, bus_dev_t &_bus) {
        info("dev_name: %s, compatible_name: %s, bus_name: %s, drv: 0x%p",
             _bus.dev_name.c_str(), _bus.compatible_name.c_str(),
             _bus.bus_name.c_str(), _bus.drv);
        return _out;
    }
};

#endif /* _BUS_DEV_H_ */
