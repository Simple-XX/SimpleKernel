
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// dev.h for Simple-XX/SimpleKernel.

#ifndef _DEV_H_
#define _DEV_H_

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
};

#endif /* _DEV_H_ */
