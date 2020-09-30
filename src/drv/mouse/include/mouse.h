
// This file is a part of MRNIU/SimpleKernel
// (https://github.com/MRNIU/SimpleKernel).
//
// mouse.h for MRNIU/SimpleKernel.
#ifndef _MOUSE_H_
#define _MOUSE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stddef.h"

#define MOU_DATA 0x60
#define MOU_CMD 0x64
#define KEYCMD_SENDTO_MOUSE 0xD4
#define MOUSECMD_ENABLE 0xF4
#define KBCMD_EN_MOUSE_INTFACE 0xA8

typedef struct mouse_desc_t {
    uint8_t  buf[3];
    uint8_t  phase;
    uint32_t x, y, btn;
} mouse_desc_t;

#ifdef __cplusplus
}
#endif

#endif /* _MOUSE_H_ */
