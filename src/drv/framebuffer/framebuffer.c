
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
#include "hardware.h"

// 宽
static uint32_t framebuffer_width;
// 高
static uint32_t framebuffer_height;
// 每行字节数
static uint32_t framebuffer_pitch;
// framebuffer 地址
static uint8_t *lfb = NULL;

static mailbox_set_physical_display_wh_data_t set_physical_display_wh_data
    __attribute__((aligned(16)));

static mailbox_set_virtual_buffer_wh_data_t set_virtual_buffer_wh_data
    __attribute__((aligned(16)));

static mailbox_set_virtual_offset_data_t set_virtual_offset_data
    __attribute__((aligned(16)));

static mailbox_set_depth_data_t set_depth_data __attribute__((aligned(16)));

static mailbox_set_pixel_order_data_t set_pixel_order_data
    __attribute__((aligned(16)));

static mailbox_allocate_buffer_data_t allocate_buffer_data
    __attribute__((aligned(16)));

static mailbox_get_pitch_data_t get_pitch_data __attribute__((aligned(16)));

void framebuffer_init(void) {
    mailbox_mail_t mail;

    // 依次填充以下字段
    // 总大小
    // 请求标识
    // tag
    // 缓冲区大小
    // 可不填写，这里用于验证
    // 数据
    // end 标记
    set_physical_display_wh_data.size =
        sizeof(mailbox_set_physical_display_wh_data_t);
    set_physical_display_wh_data.code = MAILBOX_REQUEST;
    set_physical_display_wh_data.data.common.tag =
        MAILBOX_TAG_SET_PHYSICAL_DISPLAY_WH;
    set_physical_display_wh_data.data.common.buffer_size = 8;
    set_physical_display_wh_data.data.common.code        = 8;
    set_physical_display_wh_data.data.height             = FRAMEBUFFER_HEIGHT;
    set_physical_display_wh_data.data.width              = FRAMEBUFFER_WIDTH;
    set_physical_display_wh_data.end                     = MAILBOX_TAG_END;
    // 向 mailbox 发送请求
    mail.channel = MAILBOX_CHANNEL_ARM2VC;
    mail.data    = (uint32_t)&set_physical_display_wh_data;
    mailbox_call(mail);
    if (set_physical_display_wh_data.data.common.code !=
        MAILBOX_RESPONSE_SUCCESSFUL + 8) {
        log_error("framebuffer: set_physical_display_wh.\n");
    }

    set_virtual_buffer_wh_data.size =
        sizeof(mailbox_set_virtual_buffer_wh_data_t);
    set_virtual_buffer_wh_data.code = MAILBOX_REQUEST;
    set_virtual_buffer_wh_data.data.common.tag =
        MAILBOX_TAG_SET_VIRTUAL_BUFFER_WH;
    set_virtual_buffer_wh_data.data.common.buffer_size = 8;
    set_virtual_buffer_wh_data.data.common.code        = 8;
    set_virtual_buffer_wh_data.data.height             = FRAMEBUFFER_HEIGHT;
    set_virtual_buffer_wh_data.data.width              = FRAMEBUFFER_WIDTH;
    set_virtual_buffer_wh_data.end                     = MAILBOX_TAG_END;
    mail.channel                                       = MAILBOX_CHANNEL_ARM2VC;
    mail.data = (uint32_t)&set_virtual_buffer_wh_data;
    mailbox_call(mail);
    if (set_virtual_buffer_wh_data.data.common.code !=
        MAILBOX_RESPONSE_SUCCESSFUL + 8) {
        log_error("framebuffer: set_virtual_buffer_wh.\n");
    }

    set_virtual_offset_data.size = sizeof(mailbox_set_virtual_offset_data_t);
    set_virtual_offset_data.code = MAILBOX_REQUEST;
    set_virtual_offset_data.data.common.tag = MAILBOX_TAG_SET_VIRTUAL_OFFSET;
    set_virtual_offset_data.data.common.buffer_size = 8;
    set_virtual_offset_data.data.common.code        = 8;
    set_virtual_offset_data.data.offset_x           = 0;
    set_virtual_offset_data.data.offset_y           = 0;
    set_virtual_offset_data.end                     = MAILBOX_TAG_END;
    mail.channel                                    = MAILBOX_CHANNEL_ARM2VC;
    mail.data = (uint32_t)&set_virtual_offset_data;
    mailbox_call(mail);
    if (set_virtual_offset_data.data.common.code !=
        MAILBOX_RESPONSE_SUCCESSFUL + 8) {
        log_error("framebuffer: set_virtual_offset.\n");
    }

    set_depth_data.size                    = sizeof(mailbox_set_depth_data_t);
    set_depth_data.code                    = MAILBOX_REQUEST;
    set_depth_data.data.common.tag         = MAILBOX_TAG_SET_DEPTH;
    set_depth_data.data.common.buffer_size = 4;
    set_depth_data.data.common.code        = 4;
    set_depth_data.data.depth              = 32;
    set_depth_data.end                     = MAILBOX_TAG_END;
    mail.channel                           = MAILBOX_CHANNEL_ARM2VC;
    mail.data                              = (uint32_t)&set_depth_data;
    mailbox_call(mail);
    if (set_depth_data.data.common.code != MAILBOX_RESPONSE_SUCCESSFUL + 4) {
        log_error("framebuffer: set_depth.\n");
    }

    set_pixel_order_data.size = sizeof(mailbox_set_pixel_order_data_t);
    set_pixel_order_data.code = MAILBOX_REQUEST;
    set_pixel_order_data.data.common.tag         = MAILBOX_TAG_SET_PIXEL_ORDER;
    set_pixel_order_data.data.common.buffer_size = 4;
    set_pixel_order_data.data.common.code        = 4;
    set_pixel_order_data.data.order              = FRAMEBUFFER_RGB;
    set_pixel_order_data.end                     = MAILBOX_TAG_END;
    mail.channel                                 = MAILBOX_CHANNEL_ARM2VC;
    mail.data = (uint32_t)&set_pixel_order_data;
    mailbox_call(mail);
    if (set_pixel_order_data.data.common.code !=
        MAILBOX_RESPONSE_SUCCESSFUL + 4) {
        log_error("framebuffer: set_pixel_order.\n");
    }

    allocate_buffer_data.size = sizeof(mailbox_allocate_buffer_data_t);
    allocate_buffer_data.code = MAILBOX_REQUEST;
    allocate_buffer_data.data.common.tag         = MAILBOX_TAG_ALLOCATE_BUFFER;
    allocate_buffer_data.data.common.buffer_size = 8;
    allocate_buffer_data.data.common.code        = 8;
    allocate_buffer_data.data.addr_align         = 4096;
    allocate_buffer_data.data.size               = 0;
    allocate_buffer_data.end                     = MAILBOX_TAG_END;
    mail.channel                                 = MAILBOX_CHANNEL_ARM2VC;
    mail.data = (uint32_t)&allocate_buffer_data;
    mailbox_call(mail);
    if (allocate_buffer_data.data.common.code !=
        MAILBOX_RESPONSE_SUCCESSFUL + 8) {
        log_error("framebuffer: allocate_buffer.\n");
    }

    get_pitch_data.size                    = sizeof(mailbox_get_pitch_data_t);
    get_pitch_data.code                    = MAILBOX_REQUEST;
    get_pitch_data.data.common.tag         = MAILBOX_TAG_GET_PITCH;
    get_pitch_data.data.common.buffer_size = 4;
    get_pitch_data.data.common.code        = 4;
    get_pitch_data.data.pitch              = 0;
    get_pitch_data.end                     = MAILBOX_TAG_END;
    mail.channel                           = MAILBOX_CHANNEL_ARM2VC;
    mail.data                              = (uint32_t)&get_pitch_data;
    mailbox_call(mail);
    if (get_pitch_data.data.common.code != MAILBOX_RESPONSE_SUCCESSFUL + 4) {
        log_error("framebuffer: get_pitch.\n");
        log_error("MAILBOX_RESPONSE_SUCCESSFUL: 0x%X\n",
                  MAILBOX_RESPONSE_SUCCESSFUL + 8);
        log_error("get_pitch_data.code: 0x%X\n", get_pitch_data.code);
        log_error("get_pitch_data.data.common.code: 0x%X\n",
                  get_pitch_data.data.common.code);
    }

    if (set_depth_data.data.depth == 32 &&
        (void *)allocate_buffer_data.data.addr_align != NULL) {
        framebuffer_width  = set_physical_display_wh_data.data.width;
        framebuffer_height = set_physical_display_wh_data.data.height;
        framebuffer_pitch  = get_pitch_data.data.pitch;
        // 将 GPU 地址转换为 ARM 地址
        allocate_buffer_data.data.addr_align &= 0x3FFFFFFF;
        lfb = (uint8_t *)allocate_buffer_data.data.addr_align;
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
    ptr += (framebuffer_pitch * y) + (x << 2);
    *((uint32_t *)ptr) = (b << 16) | (g << 24) | r;
    return;
}

#ifdef __cplusplus
}
#endif
