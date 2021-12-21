
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_blk.cpp for Simple-XX/SimpleKernel.

#include "stdio.h"
#include "vector"
#include "intr.h"
#include "io.h"
#include "assert.h"
#include "virtio_blk.h"
#include "virtio_queue.h"

VIRTIO_BLK::VIRTIO_BLK(void *_addr) {
    regs = (virtio_regs_t *)_addr;
    // 检查相关值
    assert(IO::get_instance().read32(&regs->magic) == MAGIC_VALUE);
    assert(IO::get_instance().read32(&regs->version) == VERSION);
    // 检查类型是否符合
    /// @todo 暂时只初始化 BLOCK_DEVICE 设备
    if (IO::get_instance().read32(&regs->device_id) != BLOCK_DEVICE) {
        err("BLOCK_DEVICE\n");
        return;
    }
    // assert(IO::get_instance().read32(&regs->device_id) == _type);
    // 初始化
    // virtio-v1.1#3.1.1
    // 重置设备
    IO::get_instance().write32(&regs->status, 0);
    // 置位 ACKNOWLEDGE
    IO::get_instance().write32(&regs->status,
                               IO::get_instance().read32(&regs->status) |
                                   DEVICE_STATUS_ACKNOWLEDGE);
    // 置位 DRIVER
    IO::get_instance().write32(&regs->status,
                               IO::get_instance().read32(&regs->status) |
                                   DEVICE_STATUS_DRIVER);
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
    IO::get_instance().write32(&regs->status,
                               IO::get_instance().read32(&regs->status) |
                                   DEVICE_STATUS_FEATURES_OK);
    // 再次读取以确认设置成功
    if ((IO::get_instance().read32(&regs->status) &
         DEVICE_STATUS_FEATURES_OK) == false) {
        // 失败则报错
        assert(0);
        return;
    }
    // 设置队列
    queues.push_back(new virtio_queue_t(8));
    add_to_device(0);
    config = (virtio_blk_config_t *)&regs->config;
#define DEBUG
#ifdef DEBUG
    // TODO: << 重载
    uint32_t i = 0;
    uint32_t j = 0;
    do {
        i = IO::get_instance().read32(&regs->config_generation);
        printf("capacity: 0x%X\n",
               IO::get_instance().read64(&config->capacity));
        printf("size_max: 0x%X\n",
               IO::get_instance().read32(&config->size_max));
        printf("seg_max: 0x%X\n", IO::get_instance().read32(&config->seg_max));
        printf("cylinders: 0x%X\n",
               IO::get_instance().read16(&config->geometry.cylinders));
        printf("heads: 0x%X\n",
               IO::get_instance().read8(&config->geometry.heads));
        printf("sectors: 0x%X\n",
               IO::get_instance().read8(&config->geometry.sectors));
        printf("blk_size: 0x%X\n",
               IO::get_instance().read32(&config->blk_size));
        printf("physical_block_exp: 0x%X\n",
               IO::get_instance().read8(&config->topology.physical_block_exp));
        printf("alignment_offset: 0x%X\n",
               IO::get_instance().read8(&config->topology.alignment_offset));
        printf("min_io_size: 0x%X\n",
               IO::get_instance().read16(&config->topology.min_io_size));
        printf("opt_io_size: 0x%X\n",
               IO::get_instance().read8(&config->topology.opt_io_size));
        printf("writeback: 0x%X\n",
               IO::get_instance().read8(&config->writeback));
        printf("max_discard_sectors: 0x%X\n",
               IO::get_instance().read32(&config->max_discard_sectors));
        printf("max_discard_seg: 0x%X\n",
               IO::get_instance().read32(&config->max_discard_seg));
        printf("discard_sector_alignment: 0x%X\n",
               IO::get_instance().read32(&config->discard_sector_alignment));
        printf("max_write_zeroes_sectors: 0x%X\n",
               IO::get_instance().read32(&config->max_write_zeroes_sectors));
        printf("max_write_zeroes_seg: 0x%X\n",
               IO::get_instance().read32(&config->max_write_zeroes_seg));
        printf("write_zeroes_may_unmap: 0x%X\n",
               IO::get_instance().read8(&config->write_zeroes_may_unmap));
        j = IO::get_instance().read32(&regs->config_generation);
    } while (i != j);
#undef DEBUG
#endif
    IO::get_instance().write32(&regs->status,
                               IO::get_instance().read32(&regs->status) |
                                   DEVICE_STATUS_DRIVER_OK);
    // 至此 virtio-blk 设备的设置就完成了
    // 允许中断
    // TODO: 这里应该通过 dtb 获取中断号
    PLIC::get_instance().set(1, true);
    printf("virtio blk init\n");
    return;
}

VIRTIO_BLK::~VIRTIO_BLK(void) {
    return;
}

// BUG: 无法读写，缓冲区/磁盘文件没有被改变
size_t VIRTIO_BLK::rw(virtio_blk_req_t &_req, void *_buf) {
    uint32_t datamode = 0;

    if (_req.type == virtio_blk_req_t::IN) {
        // mark page writeable
        datamode = virtio_queue_t::virtq_desc_t::VIRTQ_DESC_F_WRITE;
    }

    uint32_t d1 = queues.at(0)->alloc_desc(&_req);
    uint32_t d2 = queues.at(0)->alloc_desc(_buf);
    uint32_t d3 =
        queues.at(0)->alloc_desc((void *)(&_req + sizeof(virtio_blk_req_t)));
    queues.at(0)->virtq->desc[d1].len = sizeof(virtio_blk_req_t);
    queues.at(0)->virtq->desc[d1].flags =
        virtio_queue_t::virtq_desc_t::VIRTQ_DESC_F_NEXT;
    queues.at(0)->virtq->desc[d1].next = d2;

    queues.at(0)->virtq->desc[d2].len = 512;
    queues.at(0)->virtq->desc[d2].flags |=
        datamode | virtio_queue_t::virtq_desc_t::VIRTQ_DESC_F_NEXT;
    queues.at(0)->virtq->desc[d2].next = d3;

    queues.at(0)->virtq->desc[d3].len = 1;
    queues.at(0)->virtq->desc[d3].flags =
        virtio_queue_t::virtq_desc_t::VIRTQ_DESC_F_WRITE;
    queues.at(0)->virtq->desc[d3].next = 0;

    queues.at(0)->virtq->avail->ring[queues.at(0)->virtq->avail->idx %
                                     queues.at(0)->virtq->len] = d1;
    queues.at(0)->virtq->avail->idx += 1;
    IO::get_instance().write32(&regs->queue_notify, 0);
    return 0;
}

bool VIRTIO_BLK::set_features(const mystl::vector<feature_t> &_features) {
    // 首先获取硬件信息
    // 0~31 位
    IO::get_instance().write32(&regs->device_features_sel, 0);
    uint32_t device_feature_low =
        IO::get_instance().read32(&regs->device_features);
    // 32~63 位
    IO::get_instance().write32(&regs->device_features_sel, 1);
    uint32_t device_feature_high =
        IO::get_instance().read32(&regs->device_features);

    // TODO: 如果有设备不支持的 feature，需要有错误反馈
    for (auto i : _features) {
        // 如果 i.bit 大于 31，那么意味着需要 DeviceFeatures 的 32~63 位，
        // 将 DeviceFeaturesSel 置位即可
        if (i.bit > 31) {
            // 如果设备支持且需要此属性
            if ((i.status == true)) {
                if ((device_feature_high & (1 << (i.bit - 32))) == true) {
                    // 选择写高位
                    IO::get_instance().write32(&regs->device_features_sel, 1);
                    // 写入
                    IO::get_instance().write32(
                        &regs->driver_features,
                        IO::get_instance().read32(&regs->driver_features) |
                            (1 << (i.bit - 32)));
                }
                else {
                    err("Not support: %s\n", i.name.c_str());
                }
            }
        }
        else {
            // 如果设备支持且需要此属性
            if ((i.status == true)) {
                if (((device_feature_low & (1 << i.bit)) != false)) {
                    // 选择写低位
                    IO::get_instance().write32(&regs->driver_features_sel, 0);
                    // 写入
                    IO::get_instance().write32(
                        &regs->driver_features,
                        IO::get_instance().read32(&regs->driver_features) |
                            (1 << i.bit));
                }
                else {
                    err("Not support: %s\n", i.name.c_str());
                }
            }
        }
    }
    return true;
}

// virtio-v1.1#4.2.3.2
void VIRTIO_BLK::add_to_device(uint32_t _queue_sel) {
    // 选择队列号
    IO::get_instance().write32(&regs->queue_sel, _queue_sel);
    // 如果该队列已设置
    if (IO::get_instance().read32(&regs->queue_ready) == 1) {
        err("already used!\n");
        // 直接返回
        return;
    }
    // max 为零则表示不支持
    assert(IO::get_instance().read32(&regs->queue_num_max) != 0);
    // 如果长度大于允许的最大值
    if (queues.at(_queue_sel)->virtq->len >
        IO::get_instance().read32(&regs->queue_num_max)) {
        err("queue 0x%X too long!\n", _queue_sel);
        return;
    }
    // 设置长度
    IO::get_instance().write32(&regs->queue_num,
                               queues.at(_queue_sel)->virtq->len);
    // 将 queue 的物理地址写入相应寄存器
    // 写 desc 低位
    IO::get_instance().write32(&regs->queue_desc_low,
                               queues.at(_queue_sel)->virtq->phys +
                                   queues.at(_queue_sel)->off_desc);
    // 写 desc 高位
    IO::get_instance().write32(&regs->queue_desc_high, 0);
    // 写 driver 低位
    IO::get_instance().write32(&regs->queue_driver_low,
                               queues.at(_queue_sel)->virtq->phys +
                                   queues.at(_queue_sel)->off_driver);
    // 写 driver 高位
    IO::get_instance().write32(&regs->queue_driver_high, 0);
    // 写 device 低位
    IO::get_instance().write32(&regs->queue_device_low,
                               queues.at(_queue_sel)->virtq->phys +
                                   queues.at(_queue_sel)->off_device);
    // 写 device 高位
    IO::get_instance().write32(&regs->queue_device_high, 0);
    // ready 置位
    IO::get_instance().write32(&regs->queue_ready, 1);
    return;
}
