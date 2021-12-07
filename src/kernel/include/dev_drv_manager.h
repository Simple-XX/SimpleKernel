
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

#ifndef _DEV_DRV_MANAGER_H_
#define _DEV_DRV_MANAGER_H_

#include "stdint.h"
#include "vector"
#include "string"
#include "dev.h"
#include "drv.h"
#include "bus_dev.h"

// 总线 设备与内核的通信方式
// 设备 挂载在总线上的硬件，总线是挂载在 null 上的一种设备
// 驱动 驱动总线/设备的代码
// 设备与驱动管理

/**
 * @brief 设备&驱动管理
 */
class DEV_DRV_MANAGER {
private:
    /// @todo 总线驱动管理
    /**
     * @brief 总线向量
     */
    mystl::vector<bus_dev_t *> buss;

    /**
     * @brief 输出总线列表
     */
    void show(void) const;

    /**
     * @brief 初始化所有总线
     * @return true             成功
     * @return false            失败
     */
    bool buss_init(void);

protected:
public:
    DEV_DRV_MANAGER(void);
    ~DEV_DRV_MANAGER(void);

    /**
     * @brief 获取单例
     * @return DEV_DRV_MANAGER& 静态对象
     */
    static DEV_DRV_MANAGER &get_instance(void);

    /**
     * @brief 初始化
     * @return true            成功
     * @return false           失败
     */
    bool init(void);

    /**
     * @brief 添加总线
     * @param  _bus             要添加的总线
     * @return true             成功
     * @return false            失败
     */
    bool add_bus(bus_dev_t &_bus);
};

#endif /* _DEV_DRV_MANAGER_H_ */
