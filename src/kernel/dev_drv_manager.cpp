
/**
 * @file dev_drv_manager.cpp
 * @brief 设备&驱动管理实现
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

#include "dev_drv_manager.h"
#include "boot_info.h"
#include "bus_device.h"
#include "cstdio"
#include "virtio_dev.h"
#include "virtio_mmio_drv.h"

void virtio_intr_handler(void) {
    warn("virtio irq handler.\n");
    return;
}

void DEV_DRV_MANAGER::show(void) const {
    info("bus count: 0x%X\n", buss.size());
    for (auto i : buss) {
        std::cout << *i << std::endl;
    }
    return;
}

DEV_DRV_MANAGER::DEV_DRV_MANAGER(void) {
    return;
}

DEV_DRV_MANAGER::~DEV_DRV_MANAGER(void) {
    for (auto i : buss) {
        delete i;
    }
    return;
}

DEV_DRV_MANAGER& DEV_DRV_MANAGER::get_instance(void) {
    /// 定义全局 DEV_DRV_MANAGER 对象
    static DEV_DRV_MANAGER dev_drv_manager;
    return dev_drv_manager;
}

/// @todo 设备管理应该更加抽象，这里只是个勉强能用的
bool DEV_DRV_MANAGER::init(void) {
    // #if defined(__riscv)
    //  获取 virtio 设备信息
    resource_t virtio_mmio_dev_resources[8];
    // 获取信息
    auto       virtio_mmio_dev_resources_count
      = BOOT_INFO::find_via_prefix("virtio_mmio@", virtio_mmio_dev_resources);
    // 转换为向量
    auto virtio_mmio_dev_resources_vector
      = new mystl::vector<resource_t>(virtio_mmio_dev_resources,
                                      virtio_mmio_dev_resources
                                        + virtio_mmio_dev_resources_count);
    // 添加 virtio 总线
    auto bus = new bus_device_t("virtio_mmio_bus");
    add_bus(*bus);
    // 每个 resource 对应一个设备
    for (auto i : *virtio_mmio_dev_resources_vector) {
        // 设置每个设备的名称与驱动名
        auto virtio_dev = new virtio_device_t(i);
        // 添加到总线的设备向量
        bus->add_device(virtio_dev);
    }
    // 将驱动注册到 bus
    register_call_back(bus, virtio_mmio_drv_t::NAME, virtio_mmio_drv_t);
    show();
    // #endif
    info("device and driver manager init.\n");
    return true;
}

bool DEV_DRV_MANAGER::add_bus(bus_device_t& _bus) {
    buss.push_back(&_bus);
    return true;
}

device_base_t* DEV_DRV_MANAGER::get_dev_via_intr_no(uint8_t _no) {
    device_base_t* res = nullptr;
    for (auto i : buss) {
        res = i->get_dev_via_intr_no(_no);
        if (res != nullptr) {
            break;
        }
    }
    return res;
}
