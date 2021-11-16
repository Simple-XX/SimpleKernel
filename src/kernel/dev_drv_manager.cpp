
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

bool DEV_DRV_MANAGER::match(dev_t &_dev, drv_t &_drv) {
    if (_dev.drv_name == _drv.name) {
        // 设置驱动
        _dev.drv = &_drv;
        return true;
    }
    else {
        return false;
    }
}

void DEV_DRV_MANAGER::show(void) const {
    info("bus count: 0x%X\n", buss.size());
    for (auto i : buss) {
        std::cout << *i << std::endl;
    }
    info("dev count: 0x%X\n", devs.size());
    for (auto i : devs) {
        std::cout << *i << std::endl;
    }
    info("drv count: 0x%X\n", drvs.size());
    for (auto i : drvs) {
        std::cout << *i << std::endl;
    }
    return;
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
    resource_t tmp;
    tmp.name = (char *)"virtio_tmp TODO";
    auto bus = new platform_bus_dev_t(tmp);
    add_bus(*bus);
    // 每个 resource 对应一个总线设备
    for (auto i : *virtio_mmio_dev_resources_vector) {
        // 设置每个设备的名称与驱动名
        auto virtio_dev = new dev_t(i);
        // 设备名
        virtio_dev->dev_name = i.name;
        // 需要的驱动名
        // TODO: 这里需要 compatible 字段
        virtio_dev->drv_name = "virtio,mmio";
        // 所属总线名
        virtio_dev->bus_name = "vortio,mmio";
        // 添加到总线向量
        add_dev(*virtio_dev);
    }

    // TODO: 添加 virtio,mmio bus 驱动
    virtio_mmio_drv_t *virtio_mmio_drv = new virtio_mmio_drv_t();
    // 到这里 virtio,mmio 总线初始化完成，下面为各个 virtio,mmio 设备进行初始化
    // 添加块设备驱动
    add_drv(*(drv_t *)virtio_mmio_drv);
    // 初始化所有设备
    for (auto i : devs) {
        if (init(*i) != true) {
            // 不成功的话输出失败信息
            warn("%s init failed\n", i->dev_name.c_str());
        }
    }
    show();
    info("device and driver manager init.\n");
    return;
}

DEV_DRV_MANAGER::~DEV_DRV_MANAGER(void) {
    for (auto i : buss) {
        delete i;
    }
    for (auto i : devs) {
        delete i;
    }
    for (auto i : drvs) {
        delete i;
    }
    return;
}

bool DEV_DRV_MANAGER::add_bus(bus_t &_bus) {
    buss.push_back(&_bus);
    // bus 也是一种设备，所以需要同时添加到设备向量中
    devs.push_back(&_bus);
    return true;
}

bool DEV_DRV_MANAGER::add_drv(drv_t &_drv) {
    drvs.push_back(&_drv);
    return true;
}

bool DEV_DRV_MANAGER::add_dev(dev_t &_dev) {
    devs.push_back(&_dev);
    return true;
}

bool DEV_DRV_MANAGER::init(dev_t &_dev) {
    // 遍历驱动列表
    for (auto i : drvs) {
        // 如果驱动匹配
        if (match(_dev, *i)) {
            // 跳出
            break;
        }
    }
    if (_dev.drv != nullptr) {
        // 执行初始化，返回执行结果
        return _dev.drv->init();
    }
    // 执行到这里说明 dev 未找到，初始化失败
    return false;
}
