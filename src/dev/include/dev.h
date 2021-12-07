
/**
 * @file dev.h
 * @brief 设备基类
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-12-01<td>MRNIU<td>迁移到 doxygen
 * </table>
 */

#ifndef _DEV_H_
#define _DEV_H_

#include "iostream"
#include "vector"
#include "string"
#include "drv.h"
#include "resource.h"

// 设备抽象
class dev_t {
private:
protected:
public:
    // 设备名
    mystl::string dev_name;
    // 设备所需驱动名
    mystl::string compatible_name;
    // 设备所属总线名
    mystl::string bus_name;
    // 设备使用的驱动
    drv_t *drv;
    // 设备所需资源
    const resource_t resource;
    dev_t(void);
    dev_t(const resource_t &_resource);
    virtual ~dev_t(void);
    friend std::ostream &operator<<(std::ostream &_out, dev_t &_dev) {
        info("dev_name: %s, compatible_name: %s, bus_name: %s, drv: 0x%p",
             _dev.dev_name.c_str(), _dev.compatible_name.c_str(),
             _dev.bus_name.c_str(), _dev.drv);
        return _out;
    }
};

#endif /* _DEV_H_ */
