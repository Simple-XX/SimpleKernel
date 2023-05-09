
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
#include "intr.h"
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

bool DEV_DRV_MANAGER::buss_init(void) {
    bool res = true;
    // 遍历所有总线
    for (auto i : buss) {
        // 遍历总线上的设备与驱动列表
        for (auto j : i->devices) {
            for (auto k : i->drvs_name) {
                // 如果匹配
                /// @todo 这里暂时写死为为每个设备分配一个驱动实例，需要改进
                if (i->match(*j, k)) {
                    // 执行初始化
                    if (j->drv->init() == true) {
                        info("%s init successful drv addr 0x8%p.\n",
                             j->dev_name.c_str(), j->drv);
                    }
                    else {
                        warn("%s init failed.\n", j->dev_name.c_str());
                    }
                }
            }
        }
        info("%s init successful.\n", i->bus_name.c_str());
    }
    return res;
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
    /// @todo 需要改进
    auto bus = new bus_device_t();
    add_bus(*bus);
    // 每个 resource 对应一个总线设备
    for (auto i : *virtio_mmio_dev_resources_vector) {
        // 设置每个设备的名称与驱动名
        auto virtio_dev      = new virtio_dev_t(i);
        // 设备名
        /// @todo 这里需要 compatible 字段 "virtio,mmio"
        virtio_dev->dev_name = i.name;
        // 添加到总线的设备向量
        bus->add_device(virtio_dev);
    }
    // 到这里 virtio,mmio 总线初始化完成，下面为各个 virtio,mmio
    // 注册用于创建驱动实例的回调函数
    register_call_back(virtio_mmio_drv_t);
    // 添加到总线的驱动向量
    bus->add_driver("virtio,mmio", "virtio_mmio_drv_t");

    bus->add_driver("virtio,mmio", "virtio_mmio_drv_t");

    // 初始化所有总线
    buss_init();
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
