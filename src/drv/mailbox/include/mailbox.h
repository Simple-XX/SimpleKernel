
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// mailbox.h for Simple-XX/SimpleKernel.

#ifndef _MAILBOX_H_
#define _MAILBOX_H_

#include "stdint.h"
#include "stdbool.h"

// See https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
// for more info

// 请求
static constexpr const uint32_t MAILBOX_REQUEST = 0;
// 应答
static constexpr const uint32_t MAILBOX_RESPONSE = 1;

// 通道
// 电源管理
static constexpr const uint32_t MAILBOX_CHANNEL_POWER = 0;
// 帧缓冲
static constexpr const uint32_t MAILBOX_CHANNEL_FRAMEBUFFER = 1;
// 虚拟UART
static constexpr const uint32_t MAILBOX_CHANNEL_VUART = 2;
// VCHIQ
static constexpr const uint32_t MAILBOX_CHANNEL_VCHIQ = 3;
// LED
static constexpr const uint32_t MAILBOX_CHANNEL_LEDS = 4;
// 按钮
static constexpr const uint32_t MAILBOX_CHANNEL_BUTTONS = 5;
// 触摸屏
static constexpr const uint32_t MAILBOX_CHANNEL_TOUCH_SCREEN = 6;
// TODO: 确定含义
static constexpr const uint32_t MAILBOX_CHANNEL_COUNT = 7;
// 属性标签(ARM to VC)
static constexpr const uint32_t MAILBOX_CHANNEL_ARM2VC = 8;
// 属性标签(VC to ARM)
static constexpr const uint32_t MAILBOX_CHANNEL_VC2ARM = 9;

// tag 标签
// 获取固件版本
static constexpr const uint32_t MAILBOX_TAG_GET_FIRMWARE_REVISION = 0x00000001;
//  获取板子类型
static constexpr const uint32_t MAILBOX_TAG_GET_BOARD_MODEL = 0x00010001;
// 获取板子版本
static constexpr const uint32_t MAILBOX_TAG_GET_BOARD_REVISION = 0x00010002;
// 获取板子 mac 地址
static constexpr const uint32_t MAILBOX_TAG_GET_BOARD_MAC_ADDRESS = 0x00010003;
// 获取串口
static constexpr const uint32_t MAILBOX_TAG_GET_BOARD_SERIAL = 0x00010004;
// 获取 arm 内存信息
static constexpr const uint32_t MAILBOX_TAG_GET_ARM_MEMORY = 0x00010005;
// 获取 vc 内存信息
static constexpr const uint32_t MAILBOX_TAG_GET_VIDEOCORE_MEMORY = 0x00010006;
// 获取时钟信息
static constexpr const uint32_t MAILBOX_TAG_GET_CLOCKS = 0x00010007;
// TODO: 确定含义
static constexpr const uint32_t MAILBOX_TAG_GET_COMMAND_LINE = 0x00050001;
// 获取 DMA 通道
static constexpr const uint32_t MAILBOX_TAG_GET_DMA_CHANNELS = 0x00060001;
// 获取电源状态
static constexpr const uint32_t MAILBOX_TAG_GET_POWER_STATE = 0x00020001;
// 获取计时器
static constexpr const uint32_t MAILBOX_TAG_GET_TIMING = 0x00020002;
// 设置电源状态
static constexpr const uint32_t MAILBOX_TAG_SET_POWER_STATE = 0x00028001;
// 获取时钟状态
static constexpr const uint32_t MAILBOX_TAG_GET_CLOCK_STATE = 0x00030001;
// 设置时钟状态
static constexpr const uint32_t MAILBOX_TAG_SET_CLOCK_STATE = 0x00038001;
// 获取时钟速率
static constexpr const uint32_t MAILBOX_TAG_GET_CLOCK_RATE = 0x00030002;
// 设置时钟速率
static constexpr const uint32_t MAILBOX_TAG_SET_CLOCK_RATE = 0x00038002;
// 获取最大时钟速率
static constexpr const uint32_t MAILBOX_TAG_GET_MAX_CLOCK_RATE = 0x00030004;
// 获取最小时钟速率
static constexpr const uint32_t MAILBOX_TAG_GET_MIN_CLOCK_RATE = 0x00030007;
// 获取 turbo
static constexpr const uint32_t MAILBOX_TAG_GET_TURBO = 0x00030009;
// 设置 turbo
static constexpr const uint32_t MAILBOX_TAG_SET_TURBO = 0x00038009;
// 获取电压
static constexpr const uint32_t MAILBOX_TAG_GET_VOLTAGE = 0x00030003;
// 设置电压
static constexpr const uint32_t MAILBOX_TAG_SET_VOLTAGE = 0x00038003;
// 获取最大电压
static constexpr const uint32_t MAILBOX_TAG_GET_MAX_VOLTAGE = 0x00030005;
// 获取最小电压
static constexpr const uint32_t MAILBOX_TAG_GET_MIN_VOLTAGE = 0x00030008;
// 获取温度
static constexpr const uint32_t MAILBOX_TAG_GET_TEMPERATURE = 0x00030006;
// 获取最大温度
static constexpr const uint32_t MAILBOX_TAG_GET_MAX_TEMPERATURE = 0x0003000A;
// 分配内存
static constexpr const uint32_t MAILBOX_TAG_ALLOCATE_MEMORY = 0x0003000C;
// 锁定内存
static constexpr const uint32_t MAILBOX_TAG_LOCK_MEMORY = 0x0003000D;
// 解锁内存
static constexpr const uint32_t MAILBOX_TAG_UNLOCK_MEMORY = 0x0003000E;
// 释放内存
static constexpr const uint32_t MAILBOX_TAG_RELEASE_MEMORY = 0x0003000F;
// 执行代码
static constexpr const uint32_t MAILBOX_TAG_EXECUTE_CODE = 0x00030010;
// TODO: 搞清楚用法
// Gets the mem_handle associated with a created dispmanx resource.
static constexpr const uint32_t MAILBOX_TAG_GET_DISPMANX_RESOURCE_MEM_HANDLER =
    0x00030014;
// 获取 EDID 时钟
static constexpr const uint32_t MAILBOX_TAG_GET_EDID_CLOCK = 0x00030020;
// 分配 frame buffer
static constexpr const uint32_t MAILBOX_TAG_ALLOCATE_BUFFER = 0x00040001;
// 释放 frame buffer
static constexpr const uint32_t MAILBOX_TAG_RELEASE_BUFFER = 0x00048001;
// 清屏
static constexpr const uint32_t MAILBOX_TAG_BLANK_SCREEN = 0x00040002;
// 获取物理显示器宽高
static constexpr const uint32_t MAILBOX_TAG_GET_PHYSICAL_DISPLAY_WH =
    0x00040003;
// 测试物理显示器宽高
static constexpr const uint32_t MAILBOX_TAG_TEST_PHYSICAL_DISPLAY_WH =
    0x00044003;
// 设置物理显示器宽高
static constexpr const uint32_t MAILBOX_TAG_SET_PHYSICAL_DISPLAY_WH =
    0x00048003;
// 获取虚拟 buffer 宽高
static constexpr const uint32_t MAILBOX_TAG_GET_VIRTUAL_BUFFER_WH = 0x00040004;
// 测试虚拟 buffer 宽高
static constexpr const uint32_t MAILBOX_TAG_TEST_VIRTUAL_BUFFER_WH = 0x00044004;
// 设置虚拟 buffer 宽高
static constexpr const uint32_t MAILBOX_TAG_SET_VIRTUAL_BUFFER_WH = 0x00048004;
// 获取深度
static constexpr const uint32_t MAILBOX_TAG_GET_DEPTH = 0x00040005;
// 测试深度
static constexpr const uint32_t MAILBOX_TAG_TEST_DEPTH = 0x00044005;
// 设置深度
static constexpr const uint32_t MAILBOX_TAG_SET_DEPTH = 0x00048005;
// 获取像素顺序
static constexpr const uint32_t MAILBOX_TAG_GET_PIXEL_ORDER = 0x00040006;
// 测试像素顺序
static constexpr const uint32_t MAILBOX_TAG_TEST_PIXEL_ORDER = 0x00044006;
// 设置像素顺序
static constexpr const uint32_t MAILBOX_TAG_SET_PIXEL_ORDER = 0x00048006;
// 获取 alpha 模式
static constexpr const uint32_t MAILBOX_TAG_GET_ALPHA_MODE = 0x00040007;
// 测试 alpha 模式
static constexpr const uint32_t MAILBOX_TAG_TEST_ALPHA_MODE = 0x00044007;
// 设置 alpha 模式
static constexpr const uint32_t MAILBOX_TAG_SET_ALPHA_MODE = 0x00048007;
// 获取每行大小
static constexpr const uint32_t MAILBOX_TAG_GET_PITCH = 0x00040008;
// 获取虚拟偏移
static constexpr const uint32_t MAILBOX_TAG_GET_VIRTUAL_OFFSET = 0x00040009;
// 测试虚拟偏移
static constexpr const uint32_t MAILBOX_TAG_TEST_VIRTUAL_OFFSET = 0x00044009;
// 设置虚拟偏移
static constexpr const uint32_t MAILBOX_TAG_SET_VIRTUAL_OFFSET = 0x00048009;
// TODO: 搞清楚用法
static constexpr const uint32_t MAILBOX_TAG_GET_OVERSCAN = 0x0004000A;
// TODO: 搞清楚用法
static constexpr const uint32_t MAILBOX_TAG_TEST_OVERSCAN = 0x0004400A;
// TODO: 搞清楚用法
static constexpr const uint32_t MAILBOX_TAG_SET_OVERSCAN = 0x0004800A;
// 获取调色板
static constexpr const uint32_t MAILBOX_TAG_GET_PALETTE = 0x0004000B;
// 测试调色板
static constexpr const uint32_t MAILBOX_TAG_TEST_PALETTE = 0x0004400B;
// 设置调色板
static constexpr const uint32_t MAILBOX_TAG_SET_PALETTE = 0x0004800B;
// 设置光标信息
static constexpr const uint32_t MAILBOX_TAG_SET_CURSOR_INFO = 0x00008010;
// 设置光标状态
static constexpr const uint32_t MAILBOX_TAG_SET_CURSOR_STATE = 0x00008011;
// 表示 tag 结束
static constexpr const uint32_t MAILBOX_TAG_END = 0x00000000;

// 通用信息
typedef struct mailbox_common {
    uint32_t tag;
    uint32_t buffer_size;
    uint32_t code;
} __attribute__((packed)) mailbox_common_t;

// MAILBOX_TAG_GET_PHYSICAL_DISPLAY_WH
typedef struct mailbox_get_physical_display_wh {
    mailbox_common_t common;
    uint32_t         width;
    uint32_t         height;
} __attribute__((packed)) mailbox_get_physical_display_wh_t;

// MAILBOX_TAG_TEST_PHYSICAL_DISPLAY_WH
typedef struct mailbox_test_physical_display_wh {
    mailbox_common_t common;
    uint32_t         width;
    uint32_t         height;
} __attribute__((packed)) mailbox_test_physical_display_wh_t;

// MAILBOX_TAG_SET_PHYSICAL_DISPLAY_WH
typedef struct mailbox_set_physical_display_wh {
    mailbox_common_t common;
    uint32_t         width;
    uint32_t         height;
} __attribute__((packed)) mailbox_set_physical_display_wh_t;

// MAILBOX_TAG_GET_VIRTUAL_BUFFER_WH
typedef struct mailbox_get_virtual_buffer_wh {
    mailbox_common_t common;
    uint32_t         width;
    uint32_t         height;
} __attribute__((packed)) mailbox_get_virtual_buffer_wh_t;

// MAILBOX_TAG_TEST_VIRTUAL_BUFFER_WH
typedef struct mailbox_test_virtual_buffer_wh {
    mailbox_common_t common;
    uint32_t         width;
    uint32_t         height;
} __attribute__((packed)) mailbox_test_virtual_buffer_wh_t;

// MAILBOX_TAG_SET_VIRTUAL_BUFFER_WH
typedef struct mailbox_set_virtual_buffer_wh {
    mailbox_common_t common;
    uint32_t         width;
    uint32_t         height;
} __attribute__((packed)) mailbox_set_virtual_buffer_wh_t;

// MAILBOX_TAG_GET_VIRTUAL_OFFSET
typedef struct mailbox_get_virtual_offset {
    mailbox_common_t common;
    uint32_t         offset_x;
    uint32_t         offset_y;
} mailbox_get_virtual_offset_t __attribute__((aligned(32)));

// MAILBOX_TAG_TEST_VIRTUAL_OFFSET
typedef struct mailbox_test_virtual_offset {
    mailbox_common_t common;
    uint32_t         offset_x;
    uint32_t         offset_y;
} mailbox_test_virtual_offset_t __attribute__((aligned(32)));

// MAILBOX_TAG_SET_VIRTUAL_OFFSET
typedef struct mailbox_set_virtual_offset {
    mailbox_common_t common;
    uint32_t         offset_x;
    uint32_t         offset_y;
} __attribute__((packed)) mailbox_set_virtual_offset_t;

// MAILBOX_TAG_GET_DEPTH
typedef struct mailbox_get_depth {
    mailbox_common_t common;
    uint32_t         depth;
} mailbox_get_depth_t __attribute__((aligned(32)));

// MAILBOX_TAG_TEST_DEPTH
typedef struct mailbox_test_depth {
    mailbox_common_t common;
    uint32_t         depth;
} mailbox_test_depth_t __attribute__((aligned(32)));

// MAILBOX_TAG_SET_DEPTH
typedef struct mailbox_set_depth {
    mailbox_common_t common;
    uint32_t         depth;
} __attribute__((packed)) mailbox_set_depth_t;

// MAILBOX_TAG_GET_PIXEL_ORDER
typedef struct mailbox_get_pixel_order {
    mailbox_common_t common;
    uint32_t         order;
} mailbox_get_pixel_order_t __attribute__((aligned(32)));

// MAILBOX_TAG_TEST_PIXEL_ORDER
typedef struct mailbox_test_pixel_order {
    mailbox_common_t common;
    uint32_t         order;
} mailbox_test_pixel_order_t __attribute__((aligned(32)));

// MAILBOX_TAG_SET_PIXEL_ORDER
typedef struct mailbox_set_pixel_order {
    mailbox_common_t common;
    uint32_t         order;
} __attribute__((packed)) mailbox_set_pixel_order_t;

// MAILBOX_TAG_ALLOCATE_BUFFER
typedef struct mailbox_allocate_buffer {
    mailbox_common_t common;
    uint32_t         addr_align;
    uint32_t         size;
} __attribute__((packed)) mailbox_allocate_buffer_t;

// MAILBOX_TAG_RELEASE_BUFFER
typedef struct mailbox_release_buffer {
    mailbox_common_t common;
} __attribute__((packed)) mailbox_release_buffer_t;

// MAILBOX_TAG_GET_PITCH
typedef struct mailbox_get_pitch {
    mailbox_common_t common;
    uint32_t         pitch;
} __attribute__((packed)) mailbox_get_pitch_t;

// 为每个 tag 创建对应类型
typedef struct mailbox_set_physical_display_wh_data {
    uint32_t                          size;
    uint32_t                          code;
    mailbox_set_physical_display_wh_t data;
    uint32_t                          end;
} __attribute__((packed)) mailbox_set_physical_display_wh_data_t;

typedef struct mailbox_set_virtual_buffer_wh_data {
    uint32_t                        size;
    uint32_t                        code;
    mailbox_set_virtual_buffer_wh_t data;
    uint32_t                        end;
} __attribute__((packed)) mailbox_set_virtual_buffer_wh_data_t;

typedef struct mailbox_set_virtual_offset_data {
    uint32_t                     size;
    uint32_t                     code;
    mailbox_set_virtual_offset_t data;
    uint32_t                     end;
} __attribute__((packed)) mailbox_set_virtual_offset_data_t;

typedef struct mailbox_set_depth_data {
    uint32_t            size;
    uint32_t            code;
    mailbox_set_depth_t data;
    uint32_t            end;
} __attribute__((packed)) mailbox_set_depth_data_t;

typedef struct mailbox_set_pixel_order_data {
    uint32_t                  size;
    uint32_t                  code;
    mailbox_set_pixel_order_t data;
    uint32_t                  end;
} __attribute__((packed)) mailbox_set_pixel_order_data_t;

typedef struct mailbox_allocate_buffer_data {
    uint32_t                  size;
    uint32_t                  code;
    mailbox_allocate_buffer_t data;
    uint32_t                  end;
} __attribute__((packed)) mailbox_allocate_buffer_data_t;

typedef struct mailbox_get_pitch_data {
    uint32_t            size;
    uint32_t            code;
    mailbox_get_pitch_t data;
    uint32_t            end;
} __attribute__((packed)) mailbox_get_pitch_data_t;

// 0-29 位未使用, 30 表示是否为空, 31 表示是否已满
typedef struct mailbox_status {
    uint32_t unused : 30;
    uint8_t  empty : 1;
    uint8_t  full : 1;
} mailbox_status_t __attribute__((aligned(32)));

typedef struct mailbox_config {

} mailbox_config_t __attribute__((aligned(32)));

class MAILBOX {
private:
    // 0-3 位为 channel, 剩余 28 位为数据
    uint32_t channel : 4;
    uint32_t data : 28;

protected:
public:
    MAILBOX(void);
    ~MAILBOX(void);
    // 读取 read 寄存器
    mailbox_mail_t get(uint8_t channel);
    // 用 peak 方式读取 read 寄存器
    mailbox_mail_t peak(uint8_t channel);
    // 写 send 寄存器
    void set(mailbox_mail_t data);
    // 读取状态
    mailbox_status_t get_status(void);
    // 写状态
    void set_status(mailbox_status_t status);
    // 读取配置
    mailbox_config_t get_config(void);
    // 写配置
    void set_config(mailbox_config_t config);
    // 执行一次收发
    mailbox_mail_t call(mailbox_mail_t mail);
};

#endif /* _MAILBOX_H_ */
