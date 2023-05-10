
/**
 * @file char_dev.h
 * @brief char 设备头文件
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-05-09
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-05-09<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#ifndef SIMPLEKERNEL_CHAR_DEV_H
#define SIMPLEKERNEL_CHAR_DEV_H

#include "cstdint"
#include "string"
#include "vector"

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

#endif /* SIMPLEKERNEL_CHAR_DEV_H */
