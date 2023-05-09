
/**
 * @file device_base.h
 * @brief 设备基类
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-12-01<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#ifndef SIMPLEKERNEL_DEVICE_BASE_H
#define SIMPLEKERNEL_DEVICE_BASE_H

#include "driver_base.h"
#include "iostream"
#include "resource.h"
#include "string"
#include "vector"

// 设备抽象
class device_base_t {
private:

protected:

public:
    /// 设备名
    mystl::string    dev_name;
    /// 设备使用的驱动
    driver_base_t*           drv;
    /// 设备所需资源
    const resource_t resource;
    /// 设备是否可用
    bool             is_ready;
    // 设备初始化
    device_base_t(void);
    device_base_t(const mystl::string& _dev_name,
                  const mystl::string& _compatible_name, driver_base_t* drv,
                  const resource_t& _resource);
    device_base_t(const resource_t& _resource);
    virtual ~device_base_t(void) = 0;

    // 设备基本操作
    // 从设备读
    virtual int          read(void* _where, void* _buf);
    // 向设备写
    virtual int          write(void* _where, void* _buf);
    // ioctl 控制
    virtual int          ioctl(uint8_t _cmd, void* _buf);
    // 获取设备状态
    virtual int          status(uint8_t _cmd);

    friend std::ostream& operator<<(std::ostream& _out, device_base_t& _dev) {
        info("dev_name: %s, drv: 0x%p",
             _dev.dev_name.c_str(), _dev.drv);
        return _out;
    }
};

#endif /* SIMPLEKERNEL_DEVICE_BASE_H */
