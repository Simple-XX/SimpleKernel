
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

// frame 信息
uint32_t width  = 0;
uint32_t height = 0;
uint32_t pitch  = 0;
uint32_t isrgb  = 0;
// framebuffer 地址
uint8_t *lfb = NULL;

void framebuffer_init(void) {
    mbox[0] = 35 * 4;
    mbox[1] = MBOX_REQUEST;

    // set phy wh
    mbox[2] = 0x48003;
    mbox[3] = 8;
    mbox[4] = 8;
    // FrameBufferInfo.width
    mbox[5] = 1024;
    // FrameBufferInfo.height
    mbox[6] = 768;

    // set virt wh
    mbox[7] = 0x48004;
    mbox[8] = 8;
    mbox[9] = 8;
    // FrameBufferInfo.virtual_width
    mbox[10] = 1024;
    // FrameBufferInfo.virtual_height
    mbox[11] = 768;

    // set virt offset
    mbox[12] = 0x48009;
    mbox[13] = 8;
    mbox[14] = 8;
    // FrameBufferInfo.x_offset
    mbox[15] = 0;
    // FrameBufferInfo.y.offset
    mbox[16] = 0;

    // set depth
    mbox[17] = 0x48005;
    mbox[18] = 4;
    mbox[19] = 4;
    // FrameBufferInfo.depth
    mbox[20] = 32;

    // set pixel order
    mbox[21] = 0x48006;
    mbox[22] = 4;
    mbox[23] = 4;
    // RGB, not BGR preferably
    mbox[24] = 1;

    // get framebuffer, gets alignment on request
    mbox[25] = 0x40001;
    mbox[26] = 8;
    mbox[27] = 8;
    // FrameBufferInfo.pointer
    mbox[28] = 4096;
    // FrameBufferInfo.size
    mbox[29] = 0;

    // get pitch
    mbox[30] = 0x40008;
    mbox[31] = 4;
    mbox[32] = 4;
    // FrameBufferInfo.pitch
    mbox[33] = 0;

    mbox[34] = MBOX_TAG_LAST;

    // this might not return exactly what we asked for, could be
    // the closest supported resolution instead
    if (mbox_call(MBOX_CH_PROP) && mbox[20] == 32 && mbox[28] != 0) {
        // TODO: 修正 pitch 数据错误的问题
        // 将 GPU 地址转换为 ARM 地址
        mbox[28] &= 0x3FFFFFFF;
        // 实际 width, 768
        width = mbox[5];
        // 实际 height, 1024
        height = mbox[6];
        // 每行大小, 4096
        pitch = mbox[33];
        // get the actual channel order
        isrgb = mbox[24];
        lfb   = (uint8_t *)mbox[28];
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
    ptr += (4096 * y) + (x << 2);
    *((uint32_t *)ptr) = (b << 16) | (g << 24) | r;
    return;
}

#ifdef __cplusplus
}
#endif
