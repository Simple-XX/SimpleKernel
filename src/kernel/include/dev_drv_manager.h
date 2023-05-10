
/**
 * @file dev_drv_manager.h
 * @brief 设备&驱动管理头文件
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

#ifndef SIMPLEKERNEL_DEV_DRV_MANAGER_H
#define SIMPLEKERNEL_DEV_DRV_MANAGER_H

#include "bus_device.h"
#include "cstdint"
#include "device_base.h"
#include "driver_base.h"
#include "string"
#include "vector"

/**
 * @brief 设备&驱动管理
 */
class DEV_DRV_MANAGER {
private:
    /**
     * @brief 总线向量
     */
    mystl::vector<bus_device_t*> buss;

    /**
     * @brief 输出总线列表
     */
    void                         show(void) const;

protected:

public:
    /**
     * @brief 构造函数
     */
    DEV_DRV_MANAGER(void);

    /**
     * @brief 析构函数
     */
    ~DEV_DRV_MANAGER(void);

    /**
     * @brief 获取单例
     * @return DEV_DRV_MANAGER& 静态对象
     */
    static DEV_DRV_MANAGER& get_instance(void);

    /**
     * @brief 初始化
     * @return true            成功
     * @return false           失败
     */
    bool                    init(void);

    /**
     * @brief 添加总线
     * @param  _bus             要添加的总线
     * @return true             成功
     * @return false            失败
     */
    bool                    add_bus(bus_device_t& _bus);

    /**
     * @brief 通过外部中断号寻找设备
     * @param  _no              外部中断号
     * @return device_base_t*   使用该中断号的设备
     */
    device_base_t*          get_dev_via_intr_no(uint8_t _no);
};

#endif /* SIMPLEKERNEL_DEV_DRV_MANAGER_H */
