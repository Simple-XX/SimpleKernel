
/**
 * @file bus_device.h
 * @brief 总线设备抽象头文件
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-12-23
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-12-01<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#ifndef SIMPLEKERNEL_BUS_DEVICE_H
#define SIMPLEKERNEL_BUS_DEVICE_H

#include "cstdint"
#include "device_base.h"
#include "resource.h"
#include "string"
#include "vector"

/**
 * @brief 总线设备抽象 总线设备是用于内核与设备通信的设备
 */
struct bus_device_t {
private:
    /**
     * @brief 匹配设备与驱动
     * @param  _dev             设备
     * @param  _drv_name        compatible-驱动名
     * @return true             成功
     * @return false            失败
     */
    virtual bool match(device_base_t& _dev, const mystl::string& _drv_name);

    bool         match(void);

protected:

public:
    /// 总线名
    mystl::string                 bus_name;
    /// 已注册设备列表
    mystl::vector<device_base_t*> devices;
    /// 已注册驱动列表 compatible-驱动名向量
    mystl::vector<mystl::string>  drivers;

    /// 用于创建虚拟总线
    bus_device_t(void) = default;
    bus_device_t(const mystl::string& _bus_name);
    virtual ~bus_device_t(void) = default;

    /**
     * @brief 添加驱动
     * @param  _compatible_name compatible 名
     * @return                  true 成功
     * @return                  false 失败
     */
    bool                 add_driver(const mystl::string& _compatible_name);

    /**
     * @brief 添加设备
     * @param  _dev             要添加的设备指针
     * @return true             成功
     * @return false            失败
     */
    bool                 add_device(device_base_t* _dev);

    /**
     * @brief 显示所有设备信息
     */
    void                 show(void) const;

    /**
     * @brief 通过外部中断号寻找设备
     * @param  _no              外部中断号
     * @return dev_t*           使用该中断号的设备
     */
    device_base_t*       get_dev_via_intr_no(uint8_t _no);

    friend std::ostream& operator<<(std::ostream& _out, bus_device_t& _bus) {
        info("bus_name: %s, devices: %d, drivers: %d", _bus.bus_name.c_str(),
             _bus.devices.size(), _bus.drivers.size());
        return _out;
    }
};

#endif /* SIMPLEKERNEL_BUS_DEVICE_H */
