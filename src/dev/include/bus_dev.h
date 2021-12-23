
/**
 * @file bus_dev.h
 * @brief 总线设备抽象头文件
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-12-23
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-12-01<td>MRNIU<td>迁移到 doxygen
 * </table>
 */

#ifndef _BUS_DEV_H_
#define _BUS_DEV_H_

#include "stdint.h"
#include "vector"
#include "string"
#include "dev.h"
#include "resource.h"

/**
 * @brief 总线设备抽象 总线设备是用于内核与设备通信的设备
 */
struct bus_dev_t : dev_t {
private:
protected:
public:
    /// 设备列表
    mystl::vector<dev_t *> devs;
    /// 驱动向量
    mystl::vector<drv_t *> drvs;
    /// compatible-驱动名向量
    mystl::vector<mystl::pair<mystl::string, mystl::string>> drvs_name;
    /// 用于创建虚拟总线
    bus_dev_t(void);
    bus_dev_t(const resource_t &_resource);
    virtual ~bus_dev_t(void) = 0;

    /**
     * @brief 添加驱动
     * @param  _drv_name        compatible 名
     * @param  _type_name       驱动名
     * @return                  true 成功
     * @return                  false 失败
     */
    bool add_drv(const mystl::string &_compatible_name,
                 const mystl::string &_drv_name);

    /**
     * @brief 添加设备
     * @param  _dev             要添加的设备指针
     * @return true             成功
     * @return false            失败
     */
    bool add_dev(dev_t *_dev);

    // 匹配设备与驱动
    /**
     * @brief 匹配设备与驱动
     * @param  _dev             设备
     * @param  _name_pair       compatible-驱动名
     * @return true             成功
     * @return false            失败
     */
    virtual bool match(dev_t                                     &_dev,
                       mystl::pair<mystl::string, mystl::string> &_name_pair);

    /**
     * @brief 显示所有设备信息
     */
    void show(void) const;

    /**
     * @brief 通过外部中断号寻找设备
     * @param  _no              外部中断号
     * @return dev_t*           使用该中断号的设备
     */
    dev_t *get_dev_via_intr_no(uint8_t _no);

    friend std::ostream &operator<<(std::ostream &_out, bus_dev_t &_bus) {
        info("dev_name: %s, compatible_name: %s, bus_name: %s, drv: 0x%p",
             _bus.dev_name.c_str(), _bus.compatible_name.c_str(),
             _bus.bus_name.c_str(), _bus.drv);
        return _out;
    }
};

#endif /* _BUS_DEV_H_ */
