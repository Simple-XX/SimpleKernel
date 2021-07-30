
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// boot_info.hpp for Simple-XX/SimpleKernel.

#ifndef _BOOT_INFO_HPP_
#define _BOOT_INFO_HPP_

#include "stdint.h"

// 声明，定义在具体的实现中
// 地址
extern "C" uintptr_t boot_info_addr;

// 由引导传递的机器信息处理
// 如 grub2 传递的 multiboot2 结构
// opensbi 传递的 dtb 结构
// 注意这部分是通过内存传递的，在重新保存之前不能被覆盖
// 实现在 dtb.cpp 或 multiboot2.cpp
class BOOT_INFO {
private:
protected:
public:
    // 长度
    static size_t boot_info_size;
    // 迭代变量
    struct iter_data_t;
    // 迭代函数
    typedef bool (*iter_fun_t)(iter_data_t *_iter_data, void *_data);
    // 迭代器
    static void iter(iter_fun_t _fun, void *_data);
    // 输出物理内存信息
    static void printf_memory(void);
    // 按名称查找
    static bool find(char *_name);
};

#endif /* _BOOT_INFO_HPP_ */
