
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// resource.h for Simple-XX/SimpleKernel.

#ifndef _RESOURCE_H_
#define _RESOURCE_H_

#include "stdint.h"
#include "string"

// 硬件设备使用的资源
// TODO: 针对所有硬件的通用抽象
struct resource_t {
private:
protected:
public:
    typedef enum : uint8_t {
        MEMORY,
        INTR,
    } type_t;
    // 资源类型
    type_t type;
    // 资源名称
    mystl::string name;
    // 当资源类型为内存时，uinon 保存内存地址
    // 当资源类型为中断号，uinon 保存中断号
    union {
        struct {
            void *start;
            void *end;
        } mem;
        uint8_t irq_no;
    };
};

#endif /* _RESOURCE_H_ */
