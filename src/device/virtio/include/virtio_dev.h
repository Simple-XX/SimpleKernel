
/**
 * @file virtio_dev.h
 * @brief virtio 设备头文件
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-12-21
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-12-01<td>MRNIU<td>迁移到 doxygen
 * </table>
 */

#ifndef SIMPLEKERNEL_VIRTIO_DEV_H
#define SIMPLEKERNEL_VIRTIO_DEV_H

#include "device_base.h"
#include "virtio_mmio_drv.h"

/**
 * @brief 所有 virtio 设备的公有属性，标准文档的 #4 之外的部分
 */
class virtio_device_t : public device_base_t {
private:

protected:

public:
    /**
     * @brief 不使用空构造函数
     */
    virtio_device_t(void) = delete;

    /**
     * @brief 构造函数
     * @param  _resource        设备使用的资源
     * @param  _drv             设备使用的驱动，默认为 nullptr
     */
    virtio_device_t(const resource_t&  _resource,
                    virtio_mmio_drv_t* _drv = nullptr);

    /**
     * @brief 使用默认析构函数
     */
    ~virtio_device_t(void) = default;
};

#endif /* SIMPLEKERNEL_VIRTIO_DEV_H */
