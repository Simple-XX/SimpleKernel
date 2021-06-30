
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// char_dev.h for Simple-XX/SimpleKernel.

#ifndef _CHAR_DEV_H_
#define _CHAR_DEV_H_

#include "stdint.h"
#include "vector"
#include "string"

// 设备抽象
class char_device_t {
private:
    // 设备名
    mystl::string name;

protected:
public:
    char_device_t(void);
    virtual ~char_device_t(void);
    // 读
    virtual uint32_t read();
    // 写
    virtual uint32_t write();
};

#endif /* _CHAR_DEV_H_ */
