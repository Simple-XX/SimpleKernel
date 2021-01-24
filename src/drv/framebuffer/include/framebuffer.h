
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// framebuffer.h for Simple-XX/SimpleKernel.

#ifndef _FRAMEBUFFER_H_
#define _FRAMEBUFFER_H_

#include "stdint.h"
#include "stddef.h"
#include "hardware.h"
#include "mailbox.h"

class FRAMEBUFFER {
private:
    static constexpr const uint32_t FRAMEBUFFER_WIDTH  = 1024;
    static constexpr const uint32_t FRAMEBUFFER_HEIGHT = 768;
    static constexpr const uint32_t FRAMEBUFFER_RGB    = 1;
    static constexpr const uint32_t FRAMEBUFFER_BGR    = 0;

    // 宽
    uint32_t framebuffer_width;
    // 高
    uint32_t framebuffer_height;
    // 每行字节数
    uint32_t framebuffer_pitch;
    // framebuffer 地址
    uint8_t *lfb = nullptr;

    mailbox_set_physical_display_wh_data_t set_physical_display_wh_data
        __attribute__((aligned(16)));

    mailbox_set_virtual_buffer_wh_data_t set_virtual_buffer_wh_data
        __attribute__((aligned(16)));

    mailbox_set_virtual_offset_data_t set_virtual_offset_data
        __attribute__((aligned(16)));

    mailbox_set_depth_data_t set_depth_data __attribute__((aligned(16)));

    mailbox_set_pixel_order_data_t set_pixel_order_data
        __attribute__((aligned(16)));

    mailbox_allocate_buffer_data_t allocate_buffer_data
        __attribute__((aligned(16)));

    mailbox_get_pitch_data_t get_pitch_data __attribute__((aligned(16)));
    NAILBOX                  mail;

protected:
public:
    FRAMEBUFFER(void);
    ~FRAMEBUFFER(void);
    void init(void);
    void set_pixel(uint32_t x, uint32_t y, uint32_t color);
};

#endif /* _FRAMEBUFFER_H_ */
