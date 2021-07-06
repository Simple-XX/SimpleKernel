
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

// 声明
class bus_t;

class resource_t {
private:
protected:
public:
    // 资源类型
    uint64_t type;
    // 资源名称
    mystl::string name;
    // 当资源类型为内存时，uinon 保存内存地址
    // 当资源类型为中断号，uinon 保存中断号
    union {
        struct {
            uint64_t start;
            uint64_t end;
        } mem;
        uint64_t iqr_no;
    };
};

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
    // 匹配驱动
    virtual bool match_drv(drv_t &_drv) = 0;
};

// 总线设备抽象
// 总线设备是用于内核与设备通信的设备
class bus_t : dev_t {
private:
protected:
public:
    bus_t(void);
    virtual ~bus_t(void) = 0;
};

#endif /* _DEV_H_ */
