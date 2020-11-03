
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

#define FRAMEBUFFER_WIDTH (1024UL)
#define FRAMEBUFFER_HEIGHT (768UL)
#define FRAMEBUFFER_PITCH (4096UL)
#define FRAMEBUFFER_ISRGB (1UL)

void framebuffer_init(void);

void framebuffer_set_pixel(uint32_t x, uint32_t y, uint32_t color);

#ifdef __cplusplus
}
#endif

#endif /* _FRAMEBUFFER_H_ */
