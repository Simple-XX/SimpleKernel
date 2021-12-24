
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
 * <tr><td>2021-12-23<td>MRNIU<td>新增文件
 * </table>
 */

#include "stdint.h"
#include "vector"
#include "string"
#include "virtio_queue.h"
#include "virtio_mmio_drv.h"
#include "drv.h"
#include "io.h"
#include "intr.h"
#include "dev_drv_manager.h"

// static void virtio_blk_handle_used(struct virtio_blk *dev, uint32_t usedidx)
// {
//     struct virtqueue      *virtq = dev->virtq;
//     uint32_t               desc1, desc2, desc3;
//     struct virtio_blk_req *req;

//     desc1 = virtq->used->ring[usedidx].id;
//     if (!(virtq->desc[desc1].flags & VIRTQ_DESC_F_NEXT))
//         goto bad_desc;
//     desc2 = virtq->desc[desc1].next;
//     if (!(virtq->desc[desc2].flags & VIRTQ_DESC_F_NEXT))
//         goto bad_desc;
//     desc3 = virtq->desc[desc2].next;
//     if (virtq->desc[desc1].len != VIRTIO_BLK_REQ_HEADER_SIZE ||
//         virtq->desc[desc2].len != VIRTIO_BLK_SECTOR_SIZE ||
//         virtq->desc[desc3].len != VIRTIO_BLK_REQ_FOOTER_SIZE)
//         goto bad_desc;

//     req = virtq->desc_virt[desc1];

//     virtq_free_desc(virtq, desc1);
//     virtq_free_desc(virtq, desc2);
//     virtq_free_desc(virtq, desc3);

//     switch (req->status) {
//         case VIRTIO_BLK_S_OK:
//             req->blkreq.status = BLKREQ_OK;
//             break;
//         case VIRTIO_BLK_S_IOERR:
//             req->blkreq.status = BLKREQ_ERR;
//             break;
//         default:
//             puts("Unhandled status in virtio_blk irq\n");
//             panic(NULL);
//     }

//     wait_list_awaken(&req->blkreq.wait);
//     return;
// bad_desc:
//     puts("virtio-blk received malformed descriptors\n");
//     return;
// }

/**
 * @brief mmio 中断处理
 */
void virtio_mmio_intr(uint8_t _no) {
    // 遍历设备列表，寻找驱动号对应的设备
    dev_t *dev = DEV_DRV_MANAGER::get_instance().get_dev_via_intr_no(_no);
    assert(dev != nullptr);
    virtio_mmio_drv_t *drv = (virtio_mmio_drv_t *)dev->drv;
    drv->set_intr_ack();
    size_t len = drv->get_queue_len();

    // for (size_t i = dev->virtq->seen_used; i != (dev->virtq->used->idx %
    // len);
    //      i        = wrap(i + 1, len)) {
    //     virtio_blk_handle_used(dev, i);
    // }
    // dev->virtq->seen_used = dev->virtq->used->idx % len;

    return;
}
