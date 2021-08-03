
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio.cpp for Simple-XX/SimpleKernel.

#include "stdint.h"
#include "string.h"
#include "stddef.h"
#include "stdio.h"
#include "io.h"
#include "common.h"
#include "assert.h"
#include "virtio.h"
#include "vmm.h"

virtio_mmio_dev_t::virtio_mmio_dev_t(void *_addr)
    : regs((virtio_regs_t *)_addr) {
    return;
}

virtio_mmio_dev_t::~virtio_mmio_dev_t(void) {
    return;
}

bool virtio_mmio_dev_t::is_valid(void) {
    return io.read32(&regs->device_id) != 0;
}

void virtio_mmio_dev_t::read(void) {
    return;
}

void virtio_mmio_dev_t::write(void) {
    return;
}

bool VIRTIO::set_features(const mystl::vector<feature_t> &_features) {
    // 首先获取硬件信息
    // 0~31 位
    io.write32(&regs->device_features_sel, 0);
    uint32_t device_feature_low = io.read32(&regs->device_features);
    // 32~63 位
    io.write32(&regs->device_features_sel, 1);
    uint32_t device_feature_high = io.read32(&regs->device_features);

    // TODO: 如果有设备不支持的 feature，需要有错误反馈
    for (auto i : _features) {
        // 如果 i.bit 大于 31，那么意味着需要 DeviceFeatures 的 32~63 位，
        // 将 DeviceFeaturesSel 置位即可
        if (i.bit > 31) {
            // 如果设备支持且需要此属性
            if ((i.status == true)) {
                if ((device_feature_high & (1 << (i.bit - 32))) == true) {
                    // 选择写高位
                    io.write32(&regs->device_features_sel, 1);
                    // 写入
                    io.write32(&regs->driver_features,
                               io.read32(&regs->driver_features) |
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
                    io.write32(&regs->driver_features_sel, 0);
                    // 写入
                    io.write32(&regs->driver_features,
                               io.read32(&regs->driver_features) |
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
void VIRTIO::add_to_device(uint32_t _queue_sel) {
    // 选择队列号
    io.write32(&regs->queue_sel, _queue_sel);
    // 如果该队列已设置
    if (io.read32(&regs->queue_ready) == 1) {
        err("already used!\n");
        // 直接返回
        return;
    }
    // max 为零则表示不支持
    assert(io.read32(&regs->queue_num_max) != 0);
    // 如果长度大于允许的最大值
    if (queues.at(_queue_sel)->virtq->len > io.read32(&regs->queue_num_max)) {
        err("queue 0x%X too long!\n", _queue_sel);
        return;
    }
    // 设置长度
    io.write32(&regs->queue_num, queues.at(_queue_sel)->virtq->len);
    // 将 queue 的物理地址写入相应寄存器
    // 写 desc 低位
    io.write32(&regs->queue_desc_low, queues.at(_queue_sel)->virtq->phys +
                                          queues.at(_queue_sel)->off_desc);
    // 写 desc 高位
    io.write32(&regs->queue_desc_high, 0);
    // 写 driver 低位
    io.write32(&regs->queue_driver_low, queues.at(_queue_sel)->virtq->phys +
                                            queues.at(_queue_sel)->off_driver);
    // 写 driver 高位
    io.write32(&regs->queue_driver_high, 0);
    // 写 device 低位
    io.write32(&regs->queue_device_low, queues.at(_queue_sel)->virtq->phys +
                                            queues.at(_queue_sel)->off_device);
    // 写 device 高位
    io.write32(&regs->queue_device_high, 0);
    // ready 置位
    io.write32(&regs->queue_ready, 1);
    return;
}

VIRTIO::VIRTIO(void *_addr, virt_device_type_t _type)
    : regs((virtio_mmio_dev_t::virtio_regs_t *)_addr) {
    // 检查相关值
    assert(io.read32(&regs->magic) == MAGIC_VALUE);
    assert(io.read32(&regs->version) == VERSION);
    // 检查类型是否符合
    assert(io.read32(&regs->device_id) == _type);
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
    return;
}

// VIRTIO::VIRTIO(const mystl::vector<dtb_prop_node_t *> &_props) {
//     // 设置 virtio 设备数量
//     size = _props.size();
//     virtio_mmio_t tmp;
//     for (auto i : _props) {
//         // TODO: 根据 addr_cells 与 size_cells 设置地址与长度，需要 dtb 支持
//         // 高 32 位
//         tmp.addr = (uint64_t)i->standard.reg.at(0) << 32;
//         // 低 32 位
//         tmp.addr += i->standard.reg.at(1);
//         tmp.len = (uint64_t)i->standard.reg.at(2) << 32;
//         tmp.len += i->standard.reg.at(3);
//         // 读取中断号
//         tmp.intrno = i->interrupt_device.interrupts;
// #define DEBUG
// #ifdef DEBUG
//         printf("addr: 0x%p, len: 0x%X, intrno: 0x%X\n", tmp.addr, tmp.len,
//                tmp.intrno);
// #undef DEBUG
// #endif
//         // 添加到向量中
//         mmio.push_back(tmp);
//         // TODO: 现在又缺页中断了，可以优化一下
//         // 映射内存
//         for (uint64_t addr = tmp.addr; addr < tmp.addr + tmp.len;
//              addr += COMMON::PAGE_SIZE) {
//             VMM::mmap(VMM::get_pgd(), (void *)addr, (void *)addr,
//                       VMM_PAGE_READABLE | VMM_PAGE_WRITABLE);
//         }
//         // 根据设备类型初始化相应设备
//         // 寻找有效设备，添加到向量中
//         virtio_mmio_dev_t tmp_dev((void *)tmp.addr);
//         if (tmp_dev.is_valid() == true) {
//             devs.push_back(tmp_dev);
//         }
//     }
//     return;
// }

VIRTIO::~VIRTIO(void) {
    for (auto &i : queues) {
        delete i;
    }
    return;
}
