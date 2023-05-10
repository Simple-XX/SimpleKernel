
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
    printf("virtio_blk_handle_used: 0x%X\n", _usedidx);
    virtio_mmio_drv_t*                   drv   = (virtio_mmio_drv_t*)_dev->drv;

    auto                                 virtq = drv->queue.virtq;
    uint32_t                             desc1, desc2, desc3;
    virtio_mmio_drv_t::virtio_blk_req_t* req;
    uint8_t*                             data;

    desc1 = virtq->used->ring[_usedidx].id;
    if (!(virtq->desc[desc1].flags & VIRTQ_DESC_F_NEXT)) {
        goto bad_desc;
    }
    desc2 = virtq->desc[desc1].next;
    if (!(virtq->desc[desc2].flags & VIRTQ_DESC_F_NEXT)) {
        goto bad_desc;
    }
    desc3 = virtq->desc[desc2].next;
    if (virtq->desc[desc1].len != virtio_mmio_drv_t::VIRTIO_BLK_REQ_HEADER_SIZE
        || virtq->desc[desc2].len != virtio_mmio_drv_t::VIRTIO_BLK_SECTOR_SIZE
        || virtq->desc[desc3].len
             != virtio_mmio_drv_t::VIRTIO_BLK_REQ_FOOTER_SIZE) {
        goto bad_desc;
    }

    req  = (virtio_mmio_drv_t::virtio_blk_req_t*)virtq->desc_virt[desc1];
    data = (uint8_t*)virtq->desc_virt[desc2];
    if (req->status != virtio_mmio_drv_t::virtio_blk_req_t::OK) {
        goto bad_status;
    }

    if (req->type == virtio_mmio_drv_t::virtio_blk_req_t::IN) {
        printf("virtio-blk: result: \"%s\"\n", data);
        for (size_t i = 0; i < 512; i++) {
            printf("0x%X ", data[i]);
        }
    }

    drv->queue.free_desc(desc1);
    drv->queue.free_desc(desc2);
    drv->queue.free_desc(desc3);

    return;
bad_desc:
    err("virtio-blk received malformed descriptors\n");
    return;

bad_status:
    err("virtio-blk: error in command response\n");
    return;

    // assert(_dev != nullptr);
    // virtio_mmio_drv_t*                   drv = (virtio_mmio_drv_t*)_dev->drv;
    // uint32_t                             desc1;
    // uint32_t                             desc2;
    // uint32_t                             desc3;
    // virtio_mmio_drv_t::virtio_blk_req_t* req;
    //
    // desc1 = drv->queue.virtq->used->ring[_usedidx].id;
    // if (!(drv->queue.virtq->desc[desc1].flags & VIRTQ_DESC_F_NEXT)) {
    //     err("virtio-blk received malformed descriptors\n");
    //     return;
    // }
    //
    // desc2 = drv->queue.virtq->desc[desc1].next;
    // if (!(drv->queue.virtq->desc[desc2].flags & VIRTQ_DESC_F_NEXT)) {
    //     err("virtio-blk received malformed descriptors\n");
    //     return;
    // }
    //
    // desc3 = drv->queue.virtq->desc[desc2].next;
    // if (drv->queue.virtq->desc[desc1].len
    //       != virtio_mmio_drv_t::VIRTIO_BLK_REQ_HEADER_SIZE
    //     || drv->queue.virtq->desc[desc2].len
    //          != virtio_mmio_drv_t::VIRTIO_BLK_SECTOR_SIZE
    //     || drv->queue.virtq->desc[desc3].len
    //          != virtio_mmio_drv_t::VIRTIO_BLK_REQ_FOOTER_SIZE) {
    //     err("virtio-blk received malformed descriptors\n");
    //     return;
    // }
    //
    // req = (virtio_mmio_drv_t::virtio_blk_req_t*)
    //         drv->queue.virtq->desc_virt[desc1];
    //
    // drv->queue.free_desc(desc1);
    // drv->queue.free_desc(desc2);
    // drv->queue.free_desc(desc3);
    //
    // switch (req->status) {
    //     case virtio_mmio_drv_t::virtio_blk_req_t::OK: {
    //         info("status [%d] in virtio_blk irq\n", req->status);
    //
    //         // req->blkreq.status = BLKREQ_OK;
    //         break;
    //     }
    //     case virtio_mmio_drv_t::virtio_blk_req_t::IOERR: {
    //         info("status [%d] in virtio_blk irq\n", req->status);
    //
    //         // req->blkreq.status = BLKREQ_ERR;
    //         break;
    //     }
    //     default: {
    //         err("Unhandled status [%d] in virtio_blk irq\n", req->status);
    //         return;
    //     }
    // }

    return;
}

#define wrap(x, len) ((x) & ~(len))

/**
 * @brief mmio 中断处理
 */
void virtio_mmio_intr(uint8_t _no) {
    printf("virtio_mmio_intr: 0x%X\n", _no);
    // 遍历设备列表，寻找驱动号对应的设备
    device_base_t* dev
      = DEV_DRV_MANAGER::get_instance().get_dev_via_intr_no(_no);
    assert(dev != nullptr);
    virtio_mmio_drv_t* drv = (virtio_mmio_drv_t*)dev->drv;
    drv->set_intr_ack();
    size_t len = drv->get_queue_len();
    for (size_t i = drv->queue.virtq->seen_used;
         i != (drv->queue.virtq->used->idx % len); i = wrap(i + 1, len)) {
        virtio_blk_handle_used(dev, i);
    }

    drv->queue.virtq->seen_used = drv->queue.virtq->used->idx % len;

    return;
}
