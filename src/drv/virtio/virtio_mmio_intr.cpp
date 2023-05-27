
/**
 * @file virtio_mmio_intr.cpp
 * @brief virtio mmio 中断处理
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-12-23
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-12-23<td>Zone.N<td>新增文件
 * </table>
 */

#include "dev_drv_manager.h"
#include "string"
#include "virtio_mmio_drv.h"
#include "virtqueue.h"

static void virtio_blk_handle_used(device_base_t* _dev, uint32_t _usedidx) {
    assert(_dev != nullptr);
    virtio_mmio_drv_t*                   drv   = (virtio_mmio_drv_t*)_dev->drv;
    uint32_t                             desc1 = 0;
    uint32_t                             desc2 = 0;
    uint32_t                             desc3 = 0;
    virtio_mmio_drv_t::virtio_blk_req_t* req;

    desc1 = drv->queue.virtq->used->ring[_usedidx].id;
    if (!(drv->queue.virtq->desc[desc1].flags & VIRTQ_DESC_F_NEXT)) {
        err("virtio_blk_handle_used received malformed descriptors\n");
        return;
    }

    desc2 = drv->queue.virtq->desc[desc1].next;
    if (!(drv->queue.virtq->desc[desc2].flags & VIRTQ_DESC_F_NEXT)) {
        err("virtio_blk_handle_used received malformed descriptors\n");
        return;
    }

    desc3 = drv->queue.virtq->desc[desc2].next;
    if (drv->queue.virtq->desc[desc1].len
          != virtio_mmio_drv_t::virtio_blk_req_t::HEADER_SIZE
        || drv->queue.virtq->desc[desc2].len
             != virtio_mmio_drv_t::virtio_blk_req_t::SECTOR_SIZE
        || drv->queue.virtq->desc[desc3].len
             != virtio_mmio_drv_t::virtio_blk_req_t::FOOTER_SIZE) {
        err("virtio_blk_handle_used received malformed descriptors\n");
        return;
    }
    req = (virtio_mmio_drv_t::virtio_blk_req_t*)
            drv->queue.virtq->desc_virt[desc1];
// #define DEBUG
#ifdef DEBUG
    auto data = (uint8_t*)drv->queue.virtq->desc_virt[desc2];
    if (req->type == virtio_mmio_drv_t::virtio_blk_req_t::IN) {
        printf("virtio_blk_handle_used: result: \"%s\"\n", data);
        for (size_t i = 0; i < 512; i++) {
            printf("0x%X ", data[i]);
        }
    }
#    undef DEBUG
#endif

    switch (req->status) {
        case virtio_mmio_drv_t::virtio_blk_req_t::OK: {
            info("status [%d] in virtio_blk irq\n", req->status);
            break;
        }
        case virtio_mmio_drv_t::virtio_blk_req_t::IOERR: {
            info("status [%d] in virtio_blk irq\n", req->status);
            break;
        }
        default: {
            err("Unhandled status [%d] in virtio_blk irq\n", req->status);
            break;
        }
    }

    delete req;

    drv->queue.free_desc(desc1);
    drv->queue.free_desc(desc2);
    drv->queue.free_desc(desc3);

    return;
}

void virtio_mmio_intr(uint8_t _no) {
    printf("virtio_mmio_intr: 0x%X\n", _no);
    // 遍历设备列表，寻找驱动号对应的设备
    device_base_t* dev
      = DEV_DRV_MANAGER::get_instance().get_dev_via_intr_no(_no);
    assert(dev != nullptr);
    virtio_mmio_drv_t* drv = (virtio_mmio_drv_t*)dev->drv;
    drv->set_intr_ack();
    size_t len     = drv->get_queue_len();
    size_t rev_len = ~len;
    for (size_t i = drv->queue.virtq->seen_used;
         i != (drv->queue.virtq->used->idx % len); i = ((i + 1) & rev_len)) {
        virtio_blk_handle_used(dev, i);
    }

    drv->queue.virtq->seen_used = drv->queue.virtq->used->idx % len;

    dev->buf.valid              = true;

    return;
}
