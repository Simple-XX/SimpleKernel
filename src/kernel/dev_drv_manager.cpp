
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// dev_drv_manager.cpp for Simple-XX/SimpleKernel.

#include "stdio.h"
#include "boot_info.h"
#include "intr.h"
#include "dev_drv_manager.h"
#include "platform_bus_dev.h"
#include "platform_bus_drv.h"
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
    // 便利所有总线
    for (auto i : buss) {
        // 如果初始化成功，接着初始化总线上的设备
        if (i->drv->init() == true) {
            // 遍历总线上的设备与驱动列表
            for (auto j : i->devs) {
                for (auto k : i->drvs) {
                    // 如果匹配
                    if (i->match(*j, *k)) {
                        // 执行初始化
                        if (j->drv->init() == true) {
                            info("%s init successful.\n", j->dev_name.c_str());
                        }
                        else {
                            warn("%s init failed.\n", i->dev_name.c_str());
                        }
                    }
                }
            }
            info("%s init successful.\n", i->dev_name.c_str());
        }
        // 不成功的话输出失败信息
        else {
            warn("%s init failed\n", i->dev_name.c_str());
            res = false;
        }
    }
    return res;
}

// 设备与驱动管理
// 首先注册所有驱动
// 然后初始化 dtb，找到所有硬件
// 最后遍历硬件，寻找需要的驱动
DEV_DRV_MANAGER::DEV_DRV_MANAGER(void) {
    // TODO 设备管理应该更加抽象，这里只是个勉强能用的
    // 获取 virtio 设备信息
    resource_t virtio_mmio_dev_resources[8];
    // 获取信息
    auto virtio_mmio_dev_resources_count =
        BOOT_INFO::find_via_prefix("virtio_mmio@", virtio_mmio_dev_resources);
    // 转换为向量
    auto virtio_mmio_dev_resources_vector = new mystl::vector<resource_t>(
        virtio_mmio_dev_resources,
        virtio_mmio_dev_resources + virtio_mmio_dev_resources_count);
    // 添加 virtio 总线
    /// @todo 需要改进
    resource_t tmp;
    tmp.name = (char *)"virtio_tmp bus";
    auto bus = new platform_bus_dev_t(tmp);
    add_bus(*bus);
    // 每个 resource 对应一个总线设备
    for (auto i : *virtio_mmio_dev_resources_vector) {
        // 设置每个设备的名称与驱动名
        auto virtio_dev = new virtio_dev_t(i);
        // 设备名
        virtio_dev->dev_name = i.name;
        // 需要的驱动名
        // TODO: 这里需要 compatible 字段
        virtio_dev->drv_name = "virtio,mmio";
        // 所属总线名
        virtio_dev->bus_name = "vortio";
        // 添加到总线的设备向量
        bus->add_dev(virtio_dev);
    }

    auto virtio_mmio_drv = new virtio_mmio_drv_t();
    // 到这里 virtio,mmio 总线初始化完成，下面为各个 virtio,mmio 设备进行初始化
    // 添加到总线的驱动向量
    bus->add_drv(virtio_mmio_drv);
    // 初始化所有总线
    buss_init();
    show();
    info("device and driver manager init.\n");
    return;
}

DEV_DRV_MANAGER::~DEV_DRV_MANAGER(void) {
    for (auto i : buss) {
        delete i;
    }
    return;
}

bool DEV_DRV_MANAGER::add_bus(bus_dev_t &_bus) {
    buss.push_back(&_bus);
    return true;
}
