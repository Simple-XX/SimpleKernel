
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

VIRTIO_BLK::VIRTIO_BLK(void *_addr) : VIRTIO(_addr, BLOCK_DEVICE) {
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
    config = (virtio_blk_config *)&regs->config;
// #define DEBUG
#ifdef DEBUG
    // TODO: << 重载
    uint32_t i, j;
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
    // PLIC::set(MEMLAYOUT::VIRTIO0_IRQ, true);
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
