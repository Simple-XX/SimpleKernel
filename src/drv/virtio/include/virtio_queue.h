
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// virtio_queue.h for Simple-XX/SimpleKernel.

#ifndef _VIRTIO_QUEUE_H_
#define _VIRTIO_QUEUE_H_

#include "stdint.h"
#include "vector"
#include "virtio.h"

class virtio_queue_t {
private:
protected:
public:
    // static inline int virtq_need_event(uint16_t event_idx, uint16_t new_idx,
    //                                    uint16_t old_idx) {
    //     return (uint16_t)(new_idx - event_idx - 1) <
    //            (uint16_t)(new_idx - old_idx);
    // }

    // // Get location of event indices (only with VIRTIO_F_EVENT_IDX)
    // static inline uint16_t *virtq_used_event(virtq *vq) {
    //     // For backwards compat, used event index is at *end* of avail ring.
    //     return &vq->avail->ring[vq->num];
    // }

    // static inline uint16_t *virtq_avail_event(virtq *vq) {
    //     // For backwards compat, avail event index is at *end* of used ring.
    //     return (uint16_t *)&vq->used->ring[vq->num];
    // }
};

#endif /* _VIRTIO_QUEUE_H_ */
