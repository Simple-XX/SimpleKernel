
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
protected:
public:
    bus_t(const mystl::vector<resource_t> &_res);
    virtual ~bus_t(void) = 0;
};

#endif /* _BUS_H_ */
