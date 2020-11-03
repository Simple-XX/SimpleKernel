
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// mailbox.h for Simple-XX/SimpleKernel.

#ifndef _UART_H_
#define _UART_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

// 缓冲区
extern uint32_t mbox[36] __attribute__((aligned(16)));

// See https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
// for more info

// 判断是否为请求
#define MAILBOX_REQUEST 0

// 通道
// 电源管理
#define MAILBOX_CHANNEL_POWER 0
// 帧缓冲
#define MAILBOX_CHANNEL_FRAMEBUFFER 1
// 虚拟UART
#define MAILBOX_CHANNEL_VUART 2
// VCHIQ
#define MAILBOX_CHANNEL_VCHIQ 3
// LED
#define MAILBOX_CHANNEL_LEDS 4
// 按钮
#define MAILBOX_CHANNEL_BUTTONS 5
// 触摸屏
#define MAILBOX_CHANNEL_TOUCH_SCREEN 6
// TODO: 确定含义
#define MAILBOX_CHANNEL_COUNT 7
// 属性标签(ARM to VC)
#define MAILBOX_CHANNEL_PROP_ARM2VC 8
// 属性标签(VC to ARM)
#define MAILBOX_CHANNEL_PROP_VC2ARM 9

// tag 标签
// 获取固件版本
#define MAILBOX_TAG_GET_FIRMWARE_REVISION (0x00000001UL)
//  获取板子类型
#define MAILBOX_TAG_GET_BOARD_MODEL (0x00010001UL)
// 获取板子版本
#define MAILBOX_TAG_GET_BOARD_REVISION (0x00010002UL)
// 获取板子 mac 地址
#define MAILBOX_TAG_GET_BOARD_MAC_ADDRESS (0x00010003UL)
// 获取串口
#define MAILBOX_TAG_GET_BOARD_SERIAL (0x00010004UL)
// 获取 arm 内存信息
#define MAILBOX_TAG_GET_ARM_MEMORY (0x00010005UL)
// 获取 vc 内存信息
#define MAILBOX_TAG_GET_VIDEOCORE_MEMORY (0x00010006UL)
// 获取时钟信息
#define MAILBOX_TAG_GET_CLOCKS (0x00010007UL)
// TODO: 确定含义
#define MAILBOX_TAG_GET_COMMAND_LINE (0x00050001UL)
// 获取 DMA 通道
#define MAILBOX_TAG_GET_DMA_CHANNELS (0x00060001UL)
// 获取电源状态
#define MAILBOX_TAG_GET_POWER_STATE (0x00020001UL)
// 获取计时器
#define MAILBOX_TAG_GET_TIMING (0x00020002UL)
// 设置电源状态
#define MAILBOX_TAG_SET_POWER_STATE (0x00028001UL)
// 获取时钟状态
#define MAILBOX_TAG_GET_CLOCK_STATE (0x00030001UL)
// 设置时钟状态
#define MAILBOX_TAG_SET_CLOCK_STATE (0x00038001UL)
// 获取时钟速率
#define MAILBOX_TAG_GET_CLOCK_RATE (0x00030002UL)
// 设置时钟速率
#define MAILBOX_TAG_SET_CLOCK_RATE (0x00038002UL)
// 获取最大时钟速率
#define MAILBOX_TAG_GET_MAX_CLOCK_RATE (0x00030004UL)
// 获取最小时钟速率
#define MAILBOX_TAG_GET_MIN_CLOCK_RATE (0x00030007UL)
// 获取 turbo
#define MAILBOX_TAG_GET_TURBO (0x00030009UL)
// 设置 turbo
#define MAILBOX_TAG_SET_TURBO (0x00038009UL)
// 获取电压
#define MAILBOX_TAG_GET_VOLTAGE (0x00030003UL)
// 设置电压
#define MAILBOX_TAG_SET_VOLTAGE (0x00038003UL)
// 获取最大电压
#define MAILBOX_TAG_GET_MAX_VOLTAGE (0x00030005UL)
// 获取最小电压
#define MAILBOX_TAG_GET_MIN_VOLTAGE (0x00030008UL)
// 获取温度
#define MAILBOX_TAG_GET_TEMPERATURE (0x00030006UL)
// 获取最大温度
#define MAILBOX_TAG_GET_MAX_TEMPERATURE (0x0003000AUL)
// 分配内存
#define MAILBOX_TAG_ALLOCATE_MEMORY (0x0003000CUL)
// 锁定内存
#define MAILBOX_TAG_LOCK_MEMORY (0x0003000DUL)
// 解锁内存
#define MAILBOX_TAG_UNLOCK_MEMORY (0x0003000EUL)
// 释放内存
#define MAILBOX_TAG_RELEASE_MEMORY (0x0003000FUL)
// 执行代码
#define MAILBOX_TAG_EXECUTE_CODE (0x00030010UL)
// TODO: 搞清楚用法
// Gets the mem_handle associated with a created dispmanx resource.
#define MAILBOX_TAG_GET_DISPMANX_RESOURCE_MEM_HANDLER (0x00030014UL)
// 获取 EDID 时钟
#define MAILBOX_TAG_GET_EDID_CLOCK (0x00030020UL)
// 分配 frame buffer
#define MAILBOX_TAG_ALLOCATE_BUFFER (0x00040001UL)
// 释放 frame buffer
#define MAILBOX_TAG_RELEASE_BUFFER (0x00048001UL)
// 清屏
#define MAILBOX_TAG_BLANK_SCREEN (0x00040002UL)
// 获取物理显示器宽高
#define MAILBOX_TAG_GET_PHYSICAL_DISPLAY_WH (0x00040003UL)
// 测试物理显示器宽高
#define MAILBOX_TAG_TEST_PHYSICAL_DISPLAY_WH (0x00044003UL)
// 设置物理显示器宽高
#define MAILBOX_TAG_SET_PHYSICAL_DISPLAY_WH (0x00048003UL)
// 获取虚拟 buffer 宽高
#define MAILBOX_TAG_GET_VIRTUAL_BUFFER_WH (0x00040004UL)
// 测试虚拟 buffer 宽高
#define MAILBOX_TAG_TEST_VIRTUAL_BUFFER_WH (0x00044004UL)
// 设置虚拟 buffer 宽高
#define MAILBOX_TAG_SET_VIRTUAL_BUFFER_WH (0x00048004UL)
// 获取深度
#define MAILBOX_TAG_GET_DEPTH (0x00040005UL)
// 测试深度
#define MAILBOX_TAG_TEST_DEPTH (0x00044005UL)
// 设置深度
#define MAILBOX_TAG_SET_DEPTH (0x00048005UL)
// 获取像素顺序
#define MAILBOX_TAG_GET_PIXEL_ORDER (0x00040006UL)
// 测试像素顺序
#define MAILBOX_TAG_TEST_PIXEL_ORDER (0x00044006UL)
// 设置像素顺序
#define MAILBOX_TAG_SET_PIXEL_ORDER (0x00048006UL)
// 获取 alpha 模式
#define MAILBOX_TAG_GET_ALPHA_MODE (0x00040007UL)
// 测试 alpha 模式
#define MAILBOX_TAG_TEST_ALPHA_MODE (0x00044007UL)
// 设置 alpha 模式
#define MAILBOX_TAG_SET_ALPHA_MODE (0x00048007UL)
// 获取每行大小
#define MAILBOX_TAG_GET_PITCH (0x00040008UL)
// 获取虚拟偏移
#define MAILBOX_TAG_GET_VIRTUAL_OFFSET (0x00040009UL)
// 测试虚拟偏移
#define MAILBOX_TAG_TEST_VIRTUAL_OFFSET (0x00044009UL)
// 设置虚拟偏移
#define MAILBOX_TAG_SET_VIRTUAL_OFFSET (0x00048009UL)
// TODO: 搞清楚用法
#define MAILBOX_TAG_GET_OVERSCAN (0x0004000AUL)
// TODO: 搞清楚用法
#define MAILBOX_TAG_TEST_OVERSCAN (0x0004400AUL)
// TODO: 搞清楚用法
#define MAILBOX_TAG_SET_OVERSCAN (0x0004800AUL)
// 获取调色板
#define MAILBOX_TAG_GET_PALETTE (0x0004000BUL)
// 测试调色板
#define MAILBOX_TAG_TEST_PALETTE (0x0004400BUL)
// 设置调色板
#define MAILBOX_TAG_SET_PALETTE (0x0004800BUL)
// 设置光标信息
#define MAILBOX_TAG_SET_CURSOR_INFO (0x00008010UL)
// 设置光标状态
#define MAILBOX_TAG_SET_CURSOR_STATE (0x00008011UL)
// 表示 tag 结束
#define MAILBOX_TAG_END (0x00000000UL)

// 请求信息
typedef struct mailbox_request {
    uint32_t unused : 31;
    // 为 0 时表示为 request
    uint8_t is_request : 1;
} mailbox_request_t __attribute__((aligned(32)));

// 应答信息
typedef struct mailbox_response {
    uint32_t size : 31;
    // 为 1 时表示为 request
    uint8_t is_response : 1;
} mailbox_response_t __attribute__((aligned(32)));

// 通用信息
typedef struct mailbox_common {
    uint32_t tag;
    uint32_t size;
    union {
        mailbox_request_t  req;
        mailbox_response_t resp;
    } code;
} mailbox_common_t __attribute__((aligned(32)));

// 与 tag 对应的数据结构
// 第一类
// uint32_t
// 适用 tag:
// MAILBOX_TAG_GET_FIRMWARE_REVISION, MAILBOX_TAG_GET_BOARD_MODEL,
// MAILBOX_TAG_GET_BOARD_REVISION, MAILBOX_TAG_GET_BOARD_SERIAL,
// MAILBOX_TAG_GET_DMA_CHANNELS, MAILBOX_TAG_BLANK_SCREEN,
// MAILBOX_TAG_GET_DEPTH, MAILBOX_TAG_TEST_DEPTH, MAILBOX_TAG_SET_DEPTH,
// MAILBOX_TAG_GET_PIXEL_ORDER, MAILBOX_TAG_TEST_PIXEL_ORDER,
// MAILBOX_TAG_SET_PIXEL_ORDER, MAILBOX_TAG_GET_ALPHA_MODE,
// MAILBOX_TAG_TEST_ALPHA_MODE, MAILBOX_TAG_SET_ALPHA_MODE,
// MAILBOX_TAG_GET_PITCH
typedef struct mailbox_get_1uint32t {
    mailbox_common_t common;
    uint32_t         data;
} mailbox_get_1uint32t_t __attribute__((aligned(32)));

// 第二类
// 6*uint8_t
// 适用 tag:
// MAILBOX_TAG_GET_BOARD_MAC_ADDRESS
typedef struct mailbox_get_6uint8t {
    mailbox_common_t common;
    uint8_t          data1;
    uint8_t          data2;
    uint8_t          data3;
    uint8_t          data4;
    uint8_t          data5;
    uint8_t          data6;
} mailbox_get_6uint8t_t __attribute__((aligned(32)));

// 第三类
// uint64_t
// 适用 tag:
// MAILBOX_TAG_GET_BOARD_SERIAL
typedef struct mailbox_get_1uint64t {
    mailbox_common_t common;
    uint64_t         data;
} mailbox_get_1uint64t_t __attribute__((aligned(32)));

// 第四类
// 2*uint32_t
// 适用 tag:
// MAILBOX_TAG_GET_ARM_MEMORY, PROPERTY_TAG_GET_VIDEOCORE_MEMORY,
// MAILBOX_TAG_GET_POWER_STATE, MAILBOX_TAG_GET_TIMING,
// MAILBOX_TAG_SET_POWER_STATE, MAILBOX_TAG_GET_CLOCK_STATE,
// MAILBOX_TAG_SET_CLOCK_STATE, MAILBOX_TAG_GET_CLOCK_RATE,
// MAILBOX_TAG_GET_MAX_CLOCK_RATE, MAILBOX_TAG_GET_MIN_CLOCK_RATE,
// MAILBOX_TAG_GET_TURBO, MAILBOX_TAG_SET_TURBO, MAILBOX_TAG_GET_VOLTAGE,
// MAILBOX_TAG_SET_VOLTAGE, MAILBOX_TAG_GET_MAX_VOLTAGE,
// MAILBOX_TAG_GET_MIN_VOLTAGE, MAILBOX_TAG_GET_TEMPERATURE,
// MAILBOX_TAG_GET_MAX_TEMPERATURE, MAILBOX_TAG_LOCK_MEMORY,
// MAILBOX_TAG_UNLOCK_MEMORY, MAILBOX_TAG_RELEASE_MEMORY,
// MAILBOX_TAG_ALLOCATE_BUFFER, MAILBOX_TAG_GET_PHYSICAL_DISPLAY_WH,
// MAILBOX_TAG_TEST_PHYSICAL_DISPLAY_WH, MAILBOX_TAG_SET_PHYSICAL_DISPLAY_WH,
// MAILBOX_TAG_GET_VIRTUAL_BUFFER_WH, MAILBOX_TAG_TEST_VIRTUAL_BUFFER_WH,
// MAILBOX_TAG_SET_VIRTUAL_BUFFER_WH, MAILBOX_TAG_GET_VIRTUAL_OFFSET,
// MAILBOX_TAG_TEST_VIRTUAL_OFFSET, MAILBOX_TAG_SET_VIRTUAL_OFFSET
typedef struct mailbox_get_2uint32t {
    mailbox_common_t common;
    uint32_t         data1;
    uint32_t         data2;
} mailbox_get_2uint32t_t __attribute__((aligned(32)));

// 第五类
// n*(2*uint32_t)
// n 最大值暂取 32
// 适用 tag:
// PROPERTY_TAG_GET_CLOCKS
typedef struct mailbox_get_n2uint32t {
    mailbox_common_t common;
    struct {
        uint32_t data1;
        uint32_t data2;
    } data[32];
} mailbox_get_n2uint32t_t __attribute__((aligned(32)));

// 第六类
// n*uint8_t
// n 最大值暂取 64
// 适用 tag:
// MAILBOX_TAG_GET_COMMAND_LINE
typedef struct mailbox_get_nuint8t {
    mailbox_common_t common;
    uint8_t          data[64];
} mailbox_get_nuint8t_t __attribute__((aligned(32)));

// 第七类
// 3*uint32_t
// 适用 tag:
// MAILBOX_TAG_SET_CLOCK_RATE, MAILBOX_TAG_GET_DISPMANX_RESOURCE_MEM_HANDLER,
typedef struct mailbox_get_3uint32t {
    mailbox_common_t common;
    uint32_t         data1;
    uint32_t         data2;
    uint32_t         data3;
} mailbox_get_3uint32t_t __attribute__((aligned(32)));

// 第八类
// 4*uint32_t
// 适用 tag:
// MAILBOX_TAG_ALLOCATE_MEMORY, MAILBOX_TAG_GET_OVERSCAN,
// MAILBOX_TAG_TEST_OVERSCAN, MAILBOX_TAG_SET_OVERSCAN,
typedef struct mailbox_get_4uint32t {
    mailbox_common_t common;
    uint32_t         data1;
    uint32_t         data2;
    uint32_t         data3;
    uint32_t         data4;
} mailbox_get_4uint32t_t __attribute__((aligned(32)));

// 第九类
// 7*uint32_t
// 适用 tag:
// MAILBOX_TAG_EXECUTE_CODE, MAILBOX_TAG_SET_CURSOR_INFO
typedef struct mailbox_get_7uint32t {
    mailbox_common_t common;
    uint32_t         data1;
    uint32_t         data2;
    uint32_t         data3;
    uint32_t         data4;
    uint32_t         data5;
    uint32_t         data6;
    uint32_t         data7;
} mailbox_get_7uint32t_t __attribute__((aligned(32)));

// 第十类
// 2*uint32_t, uint8_t[128]
// 适用 tag:
// MAILBOX_TAG_GET_EDID_CLOCK
typedef struct mailbox_get_2uint32tuint8t128 {
    mailbox_common_t common;
    uint32_t         data1;
    uint32_t         data2;
    uint8_t          data3[128];
} mailbox_get_2uint32tuint8t128_t __attribute__((aligned(32)));

// 第十一类
// 0
// 适用 tag:
// MAILBOX_TAG_RELEASE_BUFFER
typedef struct mailbox_get_0 {
    mailbox_common_t common;
} mailbox_get_0_t __attribute__((aligned(32)));

// 第十二类
// uint32_t[256]
// 适用 tag:
// MAILBOX_TAG_GET_PALETTE
typedef struct mailbox_get_uint32t256 {
    mailbox_common_t common;
    uint32_t         data[256];
} mailbox_get_uint32t256_t __attribute__((aligned(32)));

// 第十三类
// 2*uint32_t, uint32_t[256], uint32_t
// 适用 tag:
// MAILBOX_TAG_TEST_PALETTE, MAILBOX_TAG_SET_PALETTE
typedef struct mailbox_get_2uint32tuint32t256uint32t {
    mailbox_common_t common;
    uint32_t         data1;
    uint32_t         data2;
    uint32_t         data3[256];
    uint32_t         data4;
} mailbox_get_2uint32tuint32t256uint32t_t __attribute__((aligned(32)));

// 第十四类
// 5*uint32_t
// 适用 tag:
// MAILBOX_TAG_SET_CURSOR_STATE
typedef struct mailbox_get_5uint32t {
    mailbox_common_t common;
    uint32_t         data1;
    uint32_t         data2;
    uint32_t         data3;
    uint32_t         data4;
    uint32_t         data5;
} mailbox_get_5uint32t_t __attribute__((aligned(32)));

// 0-3 位为 channel, 剩余 28 位为数据
typedef struct mailbox_data {
    uint8_t  channel : 4;
    uint32_t data : 28;
} mailbox_data_t __attribute__((aligned(32)));

// 0-29 位未使用, 30 表示是否为空, 31 表示是否已满
typedef struct mailbox_status {
    uint32_t unused : 30;
    uint8_t  empty : 1;
    uint8_t  full : 1;
} mailbox_status_t __attribute__((aligned(32)));

// 0-29 位未使用, 30 表示是否为空, 31 表示是否已满
typedef struct mailbox_config {

} mailbox_config_t __attribute__((aligned(32)));

// 读取 read 寄存器
mailbox_data_t mailbox_get(void);

// 用 peak 方式读取 read 寄存器
mailbox_data_t mailbox_peak(void);

// 写 send 寄存器
void mailbox_set(mailbox_data_t data);

// 读取状态
mailbox_status_t mailbox_get_status(void);

// 写状态
void mailbox_set_status(mailbox_status_t status);

// 读取配置
mailbox_config_t mailbox_get_config(void);

// 写配置
void mailbox_set_config(mailbox_config_t config);

// 发送数据
int32_t mbox_call(uint8_t ch);

#ifdef __cplusplus
}
#endif

#endif /* _UART_H_ */
