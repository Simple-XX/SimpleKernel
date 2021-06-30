
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// blk_dev.h for Simple-XX/SimpleKernel.

#ifndef _BLK_DEV_H_
#define _BLK_DEV_H_

#include "stdint.h"
#include "vector"
#include "string"

// 设备抽象
class blk_device_t {
private:
    // 设备名
    mystl::string name;

protected:
public:
    blk_device_t(void);
    virtual ~blk_device_t(void);
    // 读
    virtual uint32_t read();
    // 写
    virtual uint32_t write();
};

#endif /* _BLK_DEV_H_ */
