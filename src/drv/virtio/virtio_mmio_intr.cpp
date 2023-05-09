
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
#include "virtio_queue.h"

static void virtio_blk_handle_used(device_base_t* _dev, uint32_t _usedidx) {
    printf("virtio_blk_handle_used: 0x%X\n", _usedidx);
    assert(_dev != nullptr);
    virtio_mmio_drv_t*                   drv = (virtio_mmio_drv_t*)_dev->drv;
    uint32_t                             desc1;
    uint32_t                             desc2;
    uint32_t                             desc3;
    virtio_mmio_drv_t::virtio_blk_req_t* req;

    desc1 = drv->queues.at(0)->virtq->used->ring[_usedidx].id;
    if (!(drv->queues.at(0)->virtq->desc[desc1].flags
          & virtio_queue_t::virtq_desc_t::VIRTQ_DESC_F_NEXT)) {
        err("virtio-blk received malformed descriptors\n");
        return;
    }
    desc2 = drv->queues.at(0)->virtq->desc[desc1].next;
    if (!(drv->queues.at(0)->virtq->desc[desc2].flags
          & virtio_queue_t::virtq_desc_t::VIRTQ_DESC_F_NEXT)) {
        err("virtio-blk received malformed descriptors\n");
        return;
    }
    desc3 = drv->queues.at(0)->virtq->desc[desc2].next;
    if (drv->queues.at(0)->virtq->desc[desc1].len
          != virtio_mmio_drv_t::VIRTIO_BLK_REQ_HEADER_SIZE
        || drv->queues.at(0)->virtq->desc[desc2].len
             != virtio_mmio_drv_t::VIRTIO_BLK_SECTOR_SIZE
        || drv->queues.at(0)->virtq->desc[desc3].len
             != virtio_mmio_drv_t::VIRTIO_BLK_REQ_FOOTER_SIZE) {
        err("virtio-blk received malformed descriptors\n");
        return;
    }

    req = (virtio_mmio_drv_t::virtio_blk_req_t*)drv->queues.at(0)
            ->virtq->desc_virt[desc1];

    drv->queues.at(0)->free_desc(desc1);
    drv->queues.at(0)->free_desc(desc2);
    drv->queues.at(0)->free_desc(desc3);

    switch (req->status) {
        case virtio_mmio_drv_t::virtio_blk_req_t::OK: {
            // req->blkreq.status = BLKREQ_OK;
            break;
        }
        case virtio_mmio_drv_t::virtio_blk_req_t::IOERR: {
            // req->blkreq.status = BLKREQ_ERR;
            break;
        }
        default: {
            err("Unhandled status in virtio_blk irq\n");
            return;
        }
    }

    return;
}

#define wrap(x, len) ((x) & ~(len))

/**
 * @brief mmio 中断处理
 */
void virtio_mmio_intr(uint8_t _no) {
    printf("virtio_mmio_intr: 0x%X\n", _no);
    // 遍历设备列表，寻找驱动号对应的设备
    device_base_t* dev = DEV_DRV_MANAGER::get_instance().get_dev_via_intr_no(_no);
    assert(dev != nullptr);
    virtio_mmio_drv_t* drv = (virtio_mmio_drv_t*)dev->drv;
    drv->set_intr_ack();
    size_t len = drv->get_queue_len();
    for (size_t i = drv->queues.at(0)->virtq->seen_used;
         i != (drv->queues.at(0)->virtq->used->idx % len);
         i = wrap(i + 1, len)) {
        virtio_blk_handle_used(dev, i);
    }

    drv->queues.at(0)->virtq->seen_used
      = drv->queues.at(0)->virtq->used->idx % len;

    return;
}
