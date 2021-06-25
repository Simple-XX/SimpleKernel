
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio.h for Simple-XX/SimpleKernel.

#ifndef _VIRTIO_H_
#define _VIRTIO_H_

#include "stdint.h"
#include "virtio_queue.h"

// See
// http://docs.oasis-open.org/virtio/virtio/v1.0/csprd01/virtio-v1.0-csprd01.html#x1-530002
// for more info

// virtio 设备抽象
class VIRTIO_DEVICE {
private:
protected:
    // Device Status Field
    // virtio-v1.1#2.1
    static constexpr const uint64_t ACKNOWLEDGE        = 0x1;
    static constexpr const uint64_t DRIVER             = 0x2;
    static constexpr const uint64_t DRIVER_OK          = 0x4;
    static constexpr const uint64_t FEATURES_OK        = 0x8;
    static constexpr const uint64_t DEVICE_NEEDS_RESET = 0x40;
    static constexpr const uint64_t FAILED             = 0x80;

public:
    VIRTIO_DEVICE(void);
    ~VIRTIO_DEVICE(void);
};

class VIRTIO {
private:
    // virtio mmio 控制寄存器
    // virtio-v1.1#4.2.2
    // a Little Endian equivalent of the “virt” string: 0x74726976
    static constexpr const uint64_t MAGIC_VALUE = 0x0;
    // Device version number
    // 0x2, Legacy devices(see 4.2.4 Legacy interface) used 0x1.
    static constexpr const uint64_t VERSION = 0x4;
    // Virtio Subsystem Device ID
    static constexpr const uint64_t DEVICE_ID = 0x8;
    // 0x554D4551
    static constexpr const uint64_t VENDOR_ID           = 0xC;
    static constexpr const uint64_t DEVICE_FEATURES     = 0x10;
    static constexpr const uint64_t DEVICE_FEATURES_SEL = 0x14;
    static constexpr const uint64_t DRIVER_FEATURES     = 0x20;
    static constexpr const uint64_t DRIVER_FEATURES_SEL = 0x24;
    // select queue, write-only
    static constexpr const uint64_t QUEUE_SEL = 0x30;
    // max size of current queue, read-only
    static constexpr const uint64_t QUEUE_NUM_MAX = 0x34;
    // size of current queue, write-only
    static constexpr const uint64_t QUEUE_NUM = 0x38;
    // ready bit
    static constexpr const uint64_t QUEUE_READY = 0x44;
    // write-only
    static constexpr const uint64_t QUEUE_NOTIFY = 0x50;
    // read-only
    static constexpr const uint64_t INTERRUPT_STATUS = 0x60;
    // write-only
    static constexpr const uint64_t INTERRUPT_ACK = 0x64;
    // read/write
    static constexpr const uint64_t STATUS = 0x70;
    uint32_t                        read(uint32_t _off);
    void                            write(uint32_t _off, uint32_t _val);

protected:
public:
    VIRTIO(void);
    ~VIRTIO(void);

    /**
     * @brief 初始化磁盘，配置一些参数，详细流程可以查看
     *        virtio文档5.2节
     *        https://docs.oasis-open.org/virtio/virtio/v1.1/virtio-v1.1.pdf
     *
     */
    void VirtioInit();

    /**
     * @brief 读取磁盘的相应扇区到b->data
     */
    void VirtioRead(char *buf, int sectorno);

    /**
     *  @brief 将b->data写入到磁盘对应扇区
     */
    void VirtioWrite(char *buf, int sectorno);

    /**
     * @brief virtio中断处理函数
     *
     */
    void VirtioIntr();

    /**
     * @brief 读写磁盘
     *
     * @param buf   读写数据缓存区
     * @param write 0代表读，1代表写
     */
    void VirtioRw(char *buf, uint64_t sector, int write);
};

#endif /* _VIRTIO_H_ */
