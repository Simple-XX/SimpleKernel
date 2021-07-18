
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// dev.h for Simple-XX/SimpleKernel.

#ifndef _DEV_H_
#define _DEV_H_

#include "iostream"
#include "vector"
#include "string"
#include "drv.h"
#include "resource.h"

// 设备抽象
class dev_t {
private:
protected:
public:
    // 设备名
    mystl::string dev_name;
    // 设备所需驱动名
    mystl::string drv_name;
    // 设备所属总线名
    mystl::string bus_name;
    // 设备使用的驱动
    drv_t *drv;
    // 设备所需资源，可能不止一个
    mystl::vector<resource_t *> res;
    dev_t(const mystl::vector<resource_t *> &_res);
    virtual ~dev_t(void) = 0;
    friend std::ostream &operator<<(std::ostream &_out, dev_t &_dev) {
        printf("dev_name: %s, drv_name: %s, bus_name: %s, drv: 0x%p",
               _dev.dev_name.c_str(), _dev.drv_name.c_str(),
               _dev.bus_name.c_str(), _dev.drv);
        return _out;
    }
};

#endif /* _DEV_H_ */
