
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// dev.h for Simple-XX/SimpleKernel.

#ifndef _DEV_H_
#define _DEV_H_

#include "stdint.h"
#include "vector"
#include "string"
#include "blk_dev.h"
#include "char_dev.h"
#include "drv.h"
#include "bus.h"
#include "resource.h"

// 声明
class bus_t;

// 设备抽象
class dev_t {
private:
protected:
public:
    // 设备名
    mystl::string dev_name;
    // 设备所需驱动名
    mystl::string drv_name;
    // 设备使用的驱动
    drv_t *drv;
    // 设备所属总线
    bus_t *bus;
    // 设备所需资源，可能不止一个
    mystl::vector<resource_t *> res;
    dev_t(void);
    virtual ~dev_t(void) = 0;
};

// 总线设备抽象
// 总线设备是用于内核与设备通信的设备
struct bus_t : dev_t {
private:
protected:
public:
    bus_t(void);
    virtual ~bus_t(void) = 0;
};

#endif /* _DEV_H_ */
