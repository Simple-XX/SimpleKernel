
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
    driver_base_t*   drv;
    /// 设备所需资源
    const resource_t resource;
    /// 设备是否可用
    bool             is_ready;
    // 设备初始化

    /**
     * @brief 构造函数
     */
    device_base_t(void);

    /**
     * @brief 构造函数
     * @param  _resource        设备使用的资源
     * @param  _drv             设备使用的驱动，默认为 nullptr
     */
    device_base_t(const resource_t& _resource, driver_base_t* _drv = nullptr);

    /**
     * @brief 析构函数
     */
    virtual ~device_base_t(void);

    /**
     * @brief 从设备读
     * @param  _buf             缓冲区
     */
    virtual int          read(buf_t& _buf);

    /**
     * @brief 向设备写
     * @param  _buf             缓冲区
     */
    virtual int          write(buf_t& _buf);

    /**
     * @brief ioctl 控制
     * @param  _resource        设备使用的资源
     * @param  _buf             缓冲区，512 字节
     */
    virtual int          ioctl(uint8_t _cmd, void* _buf);

    /**
     * @brief 获取设备状态
     * @param  _cmd             指令
     */
    virtual int          status(uint8_t _cmd);

    friend std::ostream& operator<<(std::ostream& _out, device_base_t& _dev) {
        info("dev_name: %s, drv: 0x%p", _dev.dev_name.c_str(), _dev.drv);
        return _out;
    }
};

#endif /* SIMPLEKERNEL_DEVICE_BASE_H */
