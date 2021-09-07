
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// drv.h for Simple-XX/SimpleKernel.

#ifndef _DRV_H_
#define _DRV_H_

#include "stdint.h"
#include "vector"
#include "string"

// 设备驱动抽象，是所有驱动的基类
class drv_t {
private:
protected:
public:
    // 驱动名
    mystl::string name;

    drv_t(void);
    virtual ~drv_t(void) = 0;
    // 驱动操作
    virtual bool         init(void)  = 0;
    virtual void         read(void)  = 0;
    virtual void         write(void) = 0;
    friend std::ostream &operator<<(std::ostream &_out, drv_t &_drv) {
        info("drv name: %s", _drv.name.c_str());
        return _out;
    }
};

#endif /* _DRV_H_ */
