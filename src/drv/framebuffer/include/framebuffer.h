
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// framebuffer.h for Simple-XX/SimpleKernel.

#ifndef _FRAMEBUFFER_H_
#define _FRAMEBUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stddef.h"
#include "hardware.h"

void framebuffer_init(void);

void framebuffer_set_pixel(uint32_t x, uint32_t y, uint32_t color);

#ifdef __cplusplus
}
#endif

#endif /* _FRAMEBUFFER_H_ */
