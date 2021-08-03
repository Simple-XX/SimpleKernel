
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// resource.h for Simple-XX/SimpleKernel.

#ifndef _RESOURCE_H_
#define _RESOURCE_H_

#include "stdint.h"
#include "string"
#include "iostream"
#include "assert.h"

// 硬件设备使用的资源
// TODO: 针对所有硬件的通用抽象
struct resource_t {
private:
protected:
public:
    typedef enum : uint8_t {
        // 内存
        MEM,
        // 中断号
        INTR_NO,
    } type_t;
    // 资源类型
    type_t type;
    // 资源名称
    char *name;

    // 当资源类型为内存时，uinon 保存内存地址
    // 当资源类型为中断号，uinon 保存中断号
    union {
        struct {
            uintptr_t addr;
            size_t    len;
        } mem;
        uint8_t irq_no;
    };

    friend std::ostream &operator<<(std::ostream &_os, const resource_t &_res) {
        switch (_res.type) {
            case MEM: {
                printf("%s(MEM): 0x%p, 0x%p", _res.name, _res.mem.addr,
                       _res.mem.len);
                break;
            }
            case INTR_NO: {
                printf("%s(INTR): 0x%p, 0x%p\n", _res.name, _res.irq_no);
                break;
            }
            default: {
                assert(0);
                break;
            }
        }
        return _os;
    }
};

#endif /* _RESOURCE_H_ */
