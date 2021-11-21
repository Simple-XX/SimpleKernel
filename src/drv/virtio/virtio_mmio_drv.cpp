
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_mmio_drv.cpp for Simple-XX/SimpleKernel.

#include "stdint.h"
#include "vector"
#include "string"
#include "virtio_queue.h"
#include "virtio_mmio_drv.h"
#include "drv.h"
#include "io.h"
#include "intr.h"

// 设置 features
bool virtio_mmio_drv_t::set_features(
    const mystl::vector<feature_t> &_features) {
    (void)_features;
    return true;
}

// 将队列设置传递到相应寄存器
// _queue_sel: 第几个队列，从 0 开始
void virtio_mmio_drv_t::add_to_device(uint32_t _queue_sel) {
    (void)_queue_sel;
    return;
}

virtio_mmio_drv_t::virtio_mmio_drv_t(void) {
    name = "virtio,mmio";
    return;
}

virtio_mmio_drv_t::virtio_mmio_drv_t(const void *_addr) {
    (void)_addr;
    return;
}

virtio_mmio_drv_t::virtio_mmio_drv_t(const resource_t &_resource) {
    regs = (virtio_regs_t *)_resource.mem.addr;
    printf("0x%p\n",regs);
    // 检查相关值
    assert(io.read32(&regs->magic) == MAGIC_VALUE);
    assert(io.read32(&regs->version) == VERSION);
    // 检查类型是否符合
    /// @todo 暂时只初始化 BLOCK_DEVICE 设备
    if(io.read32(&regs->device_id)!=BLOCK_DEVICE){
        return;
    }
    // assert(io.read32(&regs->device_id) == _type);
    // 初始化
    // virtio-v1.1#3.1.1
    // 重置设备
    io.write32(&regs->status, 0);
    // 置位 ACKNOWLEDGE
    io.write32(&regs->status,
               io.read32(&regs->status) | DEVICE_STATUS_ACKNOWLEDGE);
    // 置位 DRIVER
    io.write32(&regs->status, io.read32(&regs->status) | DEVICE_STATUS_DRIVER);
    // 接下来设置设备相关 feature，交给特定设备进行
    // 跳转到 virtio_blk.cpp 的构造函数
    // virtio-v1.1#3.1.1
    // 设置属性
    mystl::vector<feature_t> feat;
    for (auto i : base_features) {
        feat.push_back(i);
    }
    for (auto i : blk_features) {
        feat.push_back(i);
    }
    set_features(feat);
    // 属性设置完成
    // 置位 FEATURES_OK
    io.write32(&regs->status,
               io.read32(&regs->status) | DEVICE_STATUS_FEATURES_OK);
    // 再次读取以确认设置成功
    if ((io.read32(&regs->status) & DEVICE_STATUS_FEATURES_OK) == false) {
        // 失败则报错
        assert(0);
        return;
    }
    // 设置队列
    queues.push_back(new virtio_queue_t(8));
    add_to_device(0);
    config = (virtio_blk_config *)&regs->config;
#define DEBUG
#ifdef DEBUG
    // TODO: << 重载
    uint32_t i, j;
    do {
        i = io.read32(&regs->config_generation);
        printf("capacity: 0x%X\n", io.read64(&config->capacity));
        printf("size_max: 0x%X\n", io.read32(&config->size_max));
        printf("seg_max: 0x%X\n", io.read32(&config->seg_max));
        printf("cylinders: 0x%X\n", io.read16(&config->geometry.cylinders));
        printf("heads: 0x%X\n", io.read8(&config->geometry.heads));
        printf("sectors: 0x%X\n", io.read8(&config->geometry.sectors));
        printf("blk_size: 0x%X\n", io.read32(&config->blk_size));
        printf("physical_block_exp: 0x%X\n",
               io.read8(&config->topology.physical_block_exp));
        printf("alignment_offset: 0x%X\n",
               io.read8(&config->topology.alignment_offset));
        printf("min_io_size: 0x%X\n", io.read16(&config->topology.min_io_size));
        printf("opt_io_size: 0x%X\n", io.read8(&config->topology.opt_io_size));
        printf("writeback: 0x%X\n", io.read8(&config->writeback));
        printf("max_discard_sectors: 0x%X\n",
               io.read32(&config->max_discard_sectors));
        printf("max_discard_seg: 0x%X\n", io.read32(&config->max_discard_seg));
        printf("discard_sector_alignment: 0x%X\n",
               io.read32(&config->discard_sector_alignment));
        printf("max_write_zeroes_sectors: 0x%X\n",
               io.read32(&config->max_write_zeroes_sectors));
        printf("max_write_zeroes_seg: 0x%X\n",
               io.read32(&config->max_write_zeroes_seg));
        printf("write_zeroes_may_unmap: 0x%X\n",
               io.read8(&config->write_zeroes_may_unmap));
        j = io.read32(&regs->config_generation);
    } while (i != j);
#undef DEBUG
#endif
    io.write32(&regs->status,
               io.read32(&regs->status) | DEVICE_STATUS_DRIVER_OK);
    // 至此 virtio-blk 设备的设置就完成了
    // 允许中断
    // TODO: 这里应该通过 dtb 获取中断号
    PLIC::set(1, true);
    printf("virtio blk init\n");
    return;
}

virtio_mmio_drv_t::~virtio_mmio_drv_t(void) {
    return;
}

bool virtio_mmio_drv_t::init(void) {
    return true;
}
