
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on https://github.com/bztsrc/raspi3-tutorial
// framebuffer.c for Simple-XX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdio.h"
#include "framebuffer.h"
#include "mailbox.h"

// framebuffer 地址
uint8_t *lfb = NULL;

void framebuffer_init(void) {
    mbox[0] = 35 * 4;
    mbox[1] = MAILBOX_REQUEST;

    // set phy wh
    mbox[2] = MAILBOX_TAG_SET_PHYSICAL_DISPLAY_WH;
    mbox[3] = 8;
    mbox[4] = 8;
    // FrameBufferInfo.width
    mbox[5] = FRAMEBUFFER_WIDTH;
    // FrameBufferInfo.height
    mbox[6] = FRAMEBUFFER_HEIGHT;

    // set virt wh
    mbox[7] = MAILBOX_TAG_SET_VIRTUAL_BUFFER_WH;
    mbox[8] = 8;
    mbox[9] = 8;
    // FrameBufferInfo.virtual_width
    mbox[10] = FRAMEBUFFER_WIDTH;
    // FrameBufferInfo.virtual_height
    mbox[11] = FRAMEBUFFER_HEIGHT;

    // set virt offset
    mbox[12] = MAILBOX_TAG_SET_VIRTUAL_OFFSET;
    mbox[13] = 8;
    mbox[14] = 8;
    // FrameBufferInfo.x_offset
    mbox[15] = 0;
    // FrameBufferInfo.y.offset
    mbox[16] = 0;

    // set depth
    mbox[17] = MAILBOX_TAG_SET_DEPTH;
    mbox[18] = 4;
    mbox[19] = 4;
    // FrameBufferInfo.depth
    mbox[20] = 32;

    // set pixel order
    mbox[21] = MAILBOX_TAG_SET_PIXEL_ORDER;
    mbox[22] = 4;
    mbox[23] = 4;
    // RGB, not BGR preferably
    mbox[24] = FRAMEBUFFER_ISRGB;

    // get framebuffer, gets alignment on request
    mbox[25] = MAILBOX_TAG_ALLOCATE_BUFFER;
    mbox[26] = 8;
    mbox[27] = 8;
    // FrameBufferInfo.pointer
    mbox[28] = 4096;
    // FrameBufferInfo.size
    mbox[29] = 0;

    // get pitch
    mbox[30] = MAILBOX_TAG_GET_PITCH;
    mbox[31] = 4;
    mbox[32] = 4;
    // FrameBufferInfo.pitch
    mbox[33] = 0;

    mbox[34] = MAILBOX_TAG_END;

    // this might not return exactly what we asked for, could be
    // the closest supported resolution instead
    if (mbox_call(MAILBOX_CHANNEL_PROP_ARM2VC) && mbox[20] == 32 &&
        mbox[28] != 0) {
        // 将 GPU 地址转换为 ARM 地址
        mbox[28] &= 0x3FFFFFFF;
        lfb = (uint8_t *)mbox[28];
    }
    else {
        log_error("Unable to set screen resolution to 1024x768x32\n");
    }
    return;
}

void framebuffer_set_pixel(uint32_t x, uint32_t y, uint32_t color) {
    uint8_t *ptr = lfb;
    // 换算为 RGB
    uint32_t r = (color >> 16) & 0xFF;
    uint32_t g = (color >> 8) & 0xFF;
    uint32_t b = color & 0xFF;
    // 计算位置
    ptr += (FRAMEBUFFER_PITCH * y) + (x << 2);
    *((uint32_t *)ptr) = (b << 16) | (g << 24) | r;
    return;
}

#ifdef __cplusplus
}
#endif
