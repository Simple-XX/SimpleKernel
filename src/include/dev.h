
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

// TODO
class DEV {
private:
    // 块设备列表
    mystl::vector<blk_device_t *> blk_devs;
    // 字符设备列表
    mystl::vector<char_device_t *> char_devs;

protected:
public:
    DEV(void);
    ~DEV(void);
};

#endif /* _DEV_H_ */
