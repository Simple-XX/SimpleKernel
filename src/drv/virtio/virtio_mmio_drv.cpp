
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
#include "driver_base.h"
#include "intr.h"
#include "io.h"
#include "string"
#include "vector"
#include "virtqueue.h"
#include "vmm.h"

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
    if (queue.virtq->len > IO::get_instance().read32(&regs->queue_num_max)) {
        err("queue 0x%X too long!\n", _queue_sel);
        return;
    }
    // 设置长度
    IO::get_instance().write32(&regs->queue_num, queue.virtq->len);
    // 将 queue 的物理地址写入相应寄存器
    // 写 desc 低位
    IO::get_instance().write32(&regs->queue_desc_low,
                               queue.virtq->phys + queue.off_desc);
    // 写 desc 高位
    IO::get_instance().write32(&regs->queue_desc_high, 0);
    // 写 driver 低位
    IO::get_instance().write32(&regs->queue_driver_low,
                               queue.virtq->phys + queue.off_driver);
    // 写 driver 高位
    IO::get_instance().write32(&regs->queue_driver_high, 0);
    // 写 device 低位
    IO::get_instance().write32(&regs->queue_device_low,
                               queue.virtq->phys + queue.off_device);
    // 写 device 高位
    IO::get_instance().write32(&regs->queue_device_high, 0);
    // ready 置位
    IO::get_instance().write32(&regs->queue_ready, 1);
    return;
}

extern void virtio_mmio_intr(uint8_t _no);

virtio_mmio_drv_t::virtio_mmio_drv_t(const resource_t& _resource)
    : driver_base_t(NAME) {
    // 映射内存
    VMM::get_instance().mmap(VMM::get_instance().get_pgd(), _resource.mem.addr,
                             _resource.mem.addr,
                             VMM_PAGE_READABLE | VMM_PAGE_WRITABLE);
    regs = (virtio_regs_t*)_resource.mem.addr;
    // 检查相关值
    assert(IO::get_instance().read32(&regs->magic) == MAGIC_VALUE);
    assert(IO::get_instance().read32(&regs->version) == VERSION);
    // 检查类型是否符合
    device_type = (device_type_t)IO::get_instance().read32(&regs->device_id);
    // assert(IO::get_instance().read32(&regs->device_id) == _type);
    // 初始化
    // virtio-v1.1#3.1.1
    // 重置设备
    IO::get_instance().write32(&regs->status, 0);
    // 置位 ACKNOWLEDGE
    IO::get_instance().write32(&regs->status,
                               IO::get_instance().read32(&regs->status)
                                 | DEVICE_STATUS_ACKNOWLEDGE);
    // 置位 DRIVER
    IO::get_instance().write32(&regs->status,
                               IO::get_instance().read32(&regs->status)
                                 | DEVICE_STATUS_DRIVER);
    // 接下来设置设备相关 feature，交给特定设备进行
    // 跳转到 virtio_blk.cpp 的构造函数
    // virtio-v1.1#3.1.1
    // 设置属性
    for (auto i : base_features) {
        features.push_back(i);
    }

    switch (device_type) {
        case BLOCK_DEVICE: {
            for (auto i : blk_features) {
                features.push_back(i);
            }
            break;
        }
        default: {
            warn("NOT SUPPORT device type : [%s]\n",
                 virtio_device_name[device_type]);
            return;
        }
    }

    set_features(features);
    // 属性设置完成
    // 置位 FEATURES_OK
    IO::get_instance().write32(&regs->status,
                               IO::get_instance().read32(&regs->status)
                                 | DEVICE_STATUS_FEATURES_OK);
    // 再次读取以确认设置成功
    if ((IO::get_instance().read32(&regs->status) & DEVICE_STATUS_FEATURES_OK)
        == false) {
        // 失败则报错
        assert(0);
        return;
    }

    switch (device_type) {
        case BLOCK_DEVICE: {
            // 更新设备信息指针
            blk_config = (virtio_blk_config_t*)&regs->config;
            // 设置队列
            /// @todo 多队列支持
            add_to_device(0);
// #define DEBUG
#ifdef DEBUG
            uint32_t i = 0;
            uint32_t j = 0;
            do {
                i = IO::get_instance().read32(&regs->config_generation);
                std::cout << blk_config << std::endl;
                j = IO::get_instance().read32(&regs->config_generation);
            } while (i != j);
#    undef DEBUG
#endif
            break;
        }
        default: {
            warn("NOT SUPPORT device type : [%s]\n",
                 virtio_device_name[device_type]);
            return;
        }
    }

    IO::get_instance().write32(&regs->status,
                               IO::get_instance().read32(&regs->status)
                                 | DEVICE_STATUS_DRIVER_OK);
    // 至此 virtio-blk 设备的设置就完成了
    // 注册外部中断处理函数
    PLIC::get_instance().register_externel_handler(_resource.intr_no,
                                                   virtio_mmio_intr);
    // 允许中断
    PLIC::get_instance().set(_resource.intr_no, true);

    printf("virtio mmio drv init\n");
    return;
}

size_t virtio_mmio_drv_t::blk_rw(virtio_blk_req_t& _req) {
    /// @todo 错误处理
    /// @see virtio-v1.1#5.2.6
    uint32_t mode  = 0;
    uint32_t desc1 = 0;
    uint32_t desc2 = 0;
    uint32_t desc3 = 0;

    if (_req.type == virtio_blk_req_t::IN) {
        // mark page writeable
        mode = VIRTQ_DESC_F_WRITE;
    }

    desc1 = queue.alloc_desc(&_req);
    desc2 = queue.alloc_desc(_req.data);
    desc3 = queue.alloc_desc(&_req.status);

    queue.virtq->desc[desc1].len
      = virtio_mmio_drv_t::virtio_blk_req_t::HEADER_SIZE;
    queue.virtq->desc[desc1].flags = VIRTQ_DESC_F_NEXT;
    queue.virtq->desc[desc1].next  = desc2;

    queue.virtq->desc[desc2].len
      = virtio_mmio_drv_t::virtio_blk_req_t::SECTOR_SIZE;
    queue.virtq->desc[desc2].flags |= mode | VIRTQ_DESC_F_NEXT;
    queue.virtq->desc[desc2].next   = desc3;

    queue.virtq->desc[desc3].len
      = virtio_mmio_drv_t::virtio_blk_req_t::FOOTER_SIZE;
    queue.virtq->desc[desc3].flags = VIRTQ_DESC_F_WRITE;
    queue.virtq->desc[desc3].next  = 0;

    queue.virtq->avail->ring[queue.virtq->avail->idx % queue.virtq->len]
      = desc1;
    queue.virtq->avail->idx += 1;
    IO::get_instance().write32(&regs->queue_notify, 0);
    return 0;
}

void virtio_mmio_drv_t::set_intr_ack(void) {
    IO::get_instance().write32(
      &regs->interrupt_ack, IO::get_instance().read32(&regs->interrupt_status));
    return;
}

size_t virtio_mmio_drv_t::get_queue_len(void) {
    return queue.virtq->len;
}

int virtio_mmio_drv_t::read(buf_t& _buf) {
    /// @note 在 virtio_blk_handle_used 中释放
    virtio_blk_req_t* req = new virtio_blk_req_t;
    req->type             = virtio_blk_req_t::IN;
    req->sector           = _buf.sector;
    req->data             = _buf.data;
    auto ret              = blk_rw(*req);
    _buf.valid            = true;
    return ret;
}

int virtio_mmio_drv_t::write(buf_t& _buf) {
    /// @note 在 virtio_blk_handle_used 中释放
    virtio_blk_req_t* req = new virtio_blk_req_t;
    req->type             = virtio_blk_req_t::OUT;
    req->sector           = _buf.sector;
    req->data             = _buf.data;
    auto ret              = blk_rw(*req);
    _buf.valid            = true;
    return ret;
}

int virtio_mmio_drv_t::ioctl(uint8_t _cmd, void* _buf) {
    (void)_cmd;
    (void)_buf;
    return 0;
}

int virtio_mmio_drv_t::status(uint8_t _cmd) {
    (void)_cmd;
    return 0;
}

define_call_back(virtio_mmio_drv_t);
