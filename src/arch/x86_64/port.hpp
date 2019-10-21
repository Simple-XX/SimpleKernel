
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).

// port.h for MRNIU/SimpleKernel.

#ifndef _PORT_HPP_
#define _PORT_HPP_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

// 端口写一个字节
static inline void outb(uint16_t port, uint8_t value) {
	asm volatile (
		"outb %1, %0"
		: : "dN" ( port ),
		"a" ( value )
		);
}

// 端口读一个字节
static inline uint8_t inb(uint16_t port) {
	uint8_t ret;
	asm volatile (
		"inb %1, %0"
		: "=a" ( ret )
		: "dN" ( port )
		);
	return ret;
}

// 端口读一个字
static inline uint16_t inw(uint16_t port) {
	uint16_t ret;
	asm volatile (
		"inw %1, %0"
		: "=a" ( ret )
		: "dN" ( port )
		);
	return ret;
}


#ifdef __cplusplus
}
#endif


#endif /* _PORT_HPP_ */
