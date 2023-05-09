
/**
 * @file virtio_mmio_drv.cpp
 * @brief virtio mmio 驱动实现
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-12-01<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#include "virtio_mmio_drv.h"
#include "drv.h"
#include "intr.h"
#include "io.h"
#include "string"
#include "vector"
#include "virtio_queue.h"
#include "vmm.h"

// 设置 features
void virtio_mmio_drv_t::set_features(
  const mystl::vector<feature_t>& _features) {
    // 首先获取硬件信息
    // 0~31 位
    IO::get_instance().write32(&regs->device_features_sel, 0);
    uint32_t device_feature_low
      = IO::get_instance().read32(&regs->device_features);
    // 32~63 位
    IO::get_instance().write32(&regs->device_features_sel, 1);
    uint32_t device_feature_high
      = IO::get_instance().read32(&regs->device_features);

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
                      IO::get_instance().read32(&regs->driver_features)
                        | (1 << (i.bit - 32)));
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
                      IO::get_instance().read32(&regs->driver_features)
                        | (1 << i.bit));
                }
                else {
                    err("Not support: %s\n", i.name.c_str());
                }
            }
        }
    }
    return;
}

// 将队列设置传递到相应寄存器
// _queue_sel: 第几个队列，从 0 开始
void virtio_mmio_drv_t::add_to_device(uint32_t _queue_sel) {
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
    if (queues.at(_queue_sel)->virtq->len
        > IO::get_instance().read32(&regs->queue_num_max)) {
        err("queue 0x%X too long!\n", _queue_sel);
        return;
    }
    // 设置长度
    IO::get_instance().write32(&regs->queue_num,
                               queues.at(_queue_sel)->virtq->len);
    // 将 queue 的物理地址写入相应寄存器
    // 写 desc 低位
    IO::get_instance().write32(&regs->queue_desc_low,
                               queues.at(_queue_sel)->virtq->phys
                                 + queues.at(_queue_sel)->off_desc);
    // 写 desc 高位
    IO::get_instance().write32(&regs->queue_desc_high, 0);
    // 写 driver 低位
    IO::get_instance().write32(&regs->queue_driver_low,
                               queues.at(_queue_sel)->virtq->phys
                                 + queues.at(_queue_sel)->off_driver);
    // 写 driver 高位
    IO::get_instance().write32(&regs->queue_driver_high, 0);
    // 写 device 低位
    IO::get_instance().write32(&regs->queue_device_low,
                               queues.at(_queue_sel)->virtq->phys
                                 + queues.at(_queue_sel)->off_device);
    // 写 device 高位
    IO::get_instance().write32(&regs->queue_device_high, 0);
    // ready 置位
    IO::get_instance().write32(&regs->queue_ready, 1);
    return;
}

virtio_mmio_drv_t::virtio_mmio_drv_t(void)
    : drv_t("virtio,mmio", "virtio_mmio_drv_t") {
    return;
}

virtio_mmio_drv_t::virtio_mmio_drv_t(const void* _addr)
    : drv_t("virtio,mmio", "virtio_mmio_drv_t") {
    (void)_addr;
    return;
}

extern void virtio_mmio_intr(uint8_t _no);

virtio_mmio_drv_t::virtio_mmio_drv_t(const resource_t& _resource)
    : drv_t("virtio,mmio", "virtio_mmio_drv_t") {
    regs = (virtio_regs_t*)_resource.mem.addr;
    // 映射内存
    VMM::get_instance().mmap(VMM::get_instance().get_pgd(), _resource.mem.addr,
                             _resource.mem.addr,
                             VMM_PAGE_READABLE | VMM_PAGE_WRITABLE);
    // 检查相关值
    assert(IO::get_instance().read32(&regs->magic) == MAGIC_VALUE);
    assert(IO::get_instance().read32(&regs->version) == VERSION);
    // 检查类型是否符合
    /// @todo 暂时只初始化 BLOCK_DEVICE 设备
    if (IO::get_instance().read32(&regs->device_id)
        != virtio_dev_t::BLOCK_DEVICE) {
        err("JUST BLOCK_DEVICE, return\n");
        return;
    }
    // assert(IO::get_instance().read32(&regs->device_id) == _type);
    // 初始化
    // virtio-v1.1#3.1.1
    // 重置设备
    IO::get_instance().write32(&regs->status, 0);
    // 置位 ACKNOWLEDGE
    IO::get_instance().write32(&regs->status,
                               IO::get_instance().read32(&regs->status)
                                 | virtio_dev_t::DEVICE_STATUS_ACKNOWLEDGE);
    // 置位 DRIVER
    IO::get_instance().write32(&regs->status,
                               IO::get_instance().read32(&regs->status)
                                 | virtio_dev_t::DEVICE_STATUS_DRIVER);
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
                               IO::get_instance().read32(&regs->status)
                                 | virtio_dev_t::DEVICE_STATUS_FEATURES_OK);
    // 再次读取以确认设置成功
    if ((IO::get_instance().read32(&regs->status)
         & virtio_dev_t::DEVICE_STATUS_FEATURES_OK)
        == false) {
        // 失败则报错
        assert(0);
        return;
    }
    // 设置队列
    queues.push_back(new virtio_queue_t(8));
    add_to_device(0);
    config = (virtio_blk_config_t*)&regs->config;
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
#    undef DEBUG
#endif
    IO::get_instance().write32(&regs->status,
                               IO::get_instance().read32(&regs->status)
                                 | virtio_dev_t::DEVICE_STATUS_DRIVER_OK);
    // 至此 virtio-blk 设备的设置就完成了
    // 允许中断
    PLIC::get_instance().set(_resource.intr_no, true);
    // 注册外部中断处理函数
    PLIC::get_instance().register_externel_handler(1, virtio_mmio_intr);
    printf("virtio blk init\n");
    virtio_mmio_drv_t::virtio_blk_req_t* req
      = new virtio_mmio_drv_t::virtio_blk_req_t;
    req->type   = virtio_blk_req_t::IN;
    req->sector = 0;
    void* buf   = kmalloc(512);
    memset(buf, 1, 512);
    rw(*req, buf);
    return;
}

virtio_mmio_drv_t::~virtio_mmio_drv_t(void) {
    for (auto i : queues) {
        delete i;
    }
    return;
}

bool virtio_mmio_drv_t::init(void) {
    return true;
}

size_t virtio_mmio_drv_t::rw(virtio_blk_req_t& _req, void* _buf) {
    /// @todo 错误处理
    /// @see virtio-v1.1#5.2.6
    uint32_t mode = 0;
    uint32_t d1   = 0;
    uint32_t d2   = 0;
    uint32_t d3   = 0;

    if (_req.type == virtio_blk_req_t::IN) {
        // mark page writeable
        mode = virtio_queue_t::virtq_desc_t::VIRTQ_DESC_F_WRITE;
    }

    d1 = queues.at(0)->alloc_desc(&_req);
    d2 = queues.at(0)->alloc_desc(_buf);
    d3 = queues.at(0)->alloc_desc((void*)(&_req + sizeof(virtio_blk_req_t)));

    queues.at(0)->virtq->desc[d1].len = VIRTIO_BLK_REQ_HEADER_SIZE;
    queues.at(0)->virtq->desc[d1].flags
      = virtio_queue_t::virtq_desc_t::VIRTQ_DESC_F_NEXT;
    queues.at(0)->virtq->desc[d1].next = d2;

    queues.at(0)->virtq->desc[d2].len  = VIRTIO_BLK_SECTOR_SIZE;
    queues.at(0)->virtq->desc[d2].flags
      |= mode | virtio_queue_t::virtq_desc_t::VIRTQ_DESC_F_NEXT;
    queues.at(0)->virtq->desc[d2].next = d3;

    queues.at(0)->virtq->desc[d3].len  = VIRTIO_BLK_REQ_FOOTER_SIZE;
    queues.at(0)->virtq->desc[d3].flags
      = virtio_queue_t::virtq_desc_t::VIRTQ_DESC_F_WRITE;
    queues.at(0)->virtq->desc[d3].next = 0;

    queues.at(0)->virtq->avail->ring[queues.at(0)->virtq->avail->idx
                                     % queues.at(0)->virtq->len]
      = d1;
    queues.at(0)->virtq->avail->idx += 1;
    IO::get_instance().write32(&regs->queue_notify, 0);
    return 0;
}

void virtio_mmio_drv_t::set_intr_ack(void) {
    IO::get_instance().write32(
      &regs->interrupt_ack, IO::get_instance().read32(&regs->interrupt_status));
    return;
}

size_t virtio_mmio_drv_t::get_queue_len(void) {
    return queues.at(0)->virtq->len;
}

define_call_back(virtio_mmio_drv_t)
