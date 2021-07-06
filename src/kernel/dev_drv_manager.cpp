
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// dev_drv_manager.cpp for Simple-XX/SimpleKernel.

#include "dev_drv_manager.h"
#include "virtio_blk.h"
#include "virtio_scsi.h"
#include "dtb.h"
#include "string.h"
#include "intr.h"

void virtio_intr_handler(void) {
    printf("virtio irq handler.\n");
    return;
}

DEV_DRV_MANAGER::DEV_DRV_MANAGER(void) {
    // 根据 dtb 获取硬件信息
    DTB dtb = DTB();
    // 获取 virtio 设备信息
    auto virtio_mmio = dtb.find("virtio,mmio");
    // virtio 设备
    VIRTIO virtio = VIRTIO(virtio_mmio);
    // 初始化 virtio
    // TODO: 这里的参数应该从 blk_dev 获取
    // TODO: 逻辑上应该是遍历 dtb，根据设备信息进行注册，
    // 而不是预设有什么设备主动注册
    // TODO: 在设备初始化之前，virtio queue 应该已经初始化了
    // VIRTIO_BLK * blk  = new VIRTIO_BLK((void *)0x10001000);
    // VIRTIO_SCSI *scsi = new VIRTIO_SCSI((void *)0x10002000);
    // 为 virtio 注册中断
    // PLIC::register_externel_handler(PLIC::VIRTIO0_INTR, virtio_intr_handler);
    // VIRTIO_BLK::virtio_blk_req_t *res = new VIRTIO_BLK::virtio_blk_req_t;
    // res->type                         = 1;
    // res->sector                       = 0;
    // void *buf                         = malloc(512);
    // memset(buf, 1, 512);
    // blk->rw(*res, buf);
    // for (int i = 0; i < 512; i++) {
    //     printf("buf: %X\n", ((char *)buf)[i]);
    // }

    return;
}

DEV_DRV_MANAGER::~DEV_DRV_MANAGER(void) {
    for (auto i : devs) {
        delete i;
    }
    for (auto i : drvs) {
        delete i;
    }
    return;
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
        // 如果驱动名匹配
        if (_dev.match_drv(*i)) {
            // 设置驱动
            _dev.drv = i;
            // 跳出
            break;
        }
    }
    // 执行初始化
    _dev.drv->init();
    return true;
}

// TODO
// 构造函数中对地址进行遍历，初始化不同的 io 设备
// 根据设备类型分别初始化
// virtio 中断由 virtio 统一管理
// 地址也由 virtio 统一管理
// 首先得完善 dtb 的处理
