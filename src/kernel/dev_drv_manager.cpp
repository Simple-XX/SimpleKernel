
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// dev_drv_manager.cpp for Simple-XX/SimpleKernel.

#include "stdio.h"
#include "boot_info.h"
#include "intr.h"
#include "dev_drv_manager.h"
#include "virtio_bus_dev.h"
#include "virtio_bus_drv.h"

void virtio_intr_handler(void) {
    printf("virtio irq handler.\n");
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
    printf("bus count: 0x%X\n", buss.size());
    for (auto i : buss) {
        std::cout << *i << std::endl;
    }
    printf("dev count: 0x%X\n", devs.size());
    for (auto i : devs) {
        std::cout << *i << std::endl;
    }
    printf("drv count: 0x%X\n", drvs.size());
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
    // 获取 virtio 设备信息
    auto virtio_mmio_resources =
        BOOT_INFO::find_via_path("/soc/virtio_mmio@10001000");
    // 初始化 virtio_bus_dev
    virtio_bus_dev_t *virtio_bus_dev =
        new virtio_bus_dev_t(virtio_mmio_resources.at(0));
    // 添加到设备链表中
    add_bus(*(bus_t *)virtio_bus_dev);
    // 添加 virtio_bus_dev 驱动
    virtio_bus_drv_t *virtio_bus_drv = new virtio_bus_drv_t();
    add_drv(*(drv_t *)virtio_bus_drv);
    // 初始化
    if (init(*(dev_t *)virtio_bus_dev) != true) {
        // 不成功的话输出失败信息
        err("virtio init failed\n");
    }
    // virtio 设备
    // VIRTIO virtio = VIRTIO(virtio_mmio);
    // 初始化 virtio
    // TODO: 这里的参数应该从 blk_dev 获取
    // TODO: 逻辑上应该是遍历 dtb，根据设备信息进行注册，
    // 而不是预设有什么设备主动注册
    // TODO: 在设备初始化之前，virtio queue 应该已经初始化了
    // VIRTIO_BLK * blk  = new VIRTIO_BLK((void *)0x10001000);
    // VIRTIO_SCSI *scsi = new VIRTIO_SCSI((void *)0x10002000);
    // 为 virtio 注册中断
    // PLIC::register_externel_handler(PLIC::VIRTIO0_INTR,
    // virtio_intr_handler); VIRTIO_BLK::virtio_blk_req_t *res = new
    // VIRTIO_BLK::virtio_blk_req_t; res->type                         = 1;
    // res->sector                       = 0;
    // void *buf                         = malloc(512);
    // memset(buf, 1, 512);
    // blk->rw(*res, buf);
    // for (int i = 0; i < 512; i++) {
    //     printf("buf: %X\n", ((char *)buf)[i]);
    // }
    printf("device and driver manager init.\n");
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
