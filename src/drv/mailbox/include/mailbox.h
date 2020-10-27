
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
extern volatile unsigned int mbox[36];

#define MBOX_REQUEST 0

// 通道
#define MBOX_CH_POWER 0
#define MBOX_CH_FB 1
#define MBOX_CH_VUART 2
#define MBOX_CH_VCHIQ 3
#define MBOX_CH_LEDS 4
#define MBOX_CH_BTNS 5
#define MBOX_CH_TOUCH 6
#define MBOX_CH_COUNT 7
#define MBOX_CH_PROP 8

#define MBOX_TAG_GETSERIAL 0x10004
#define MBOX_TAG_LAST 0

// 发送数据
int32_t mbox_call(uint8_t ch);

#ifdef __cplusplus
}
#endif

#endif /* _UART_H_ */
