
/**
 * @file endian.h
 * @brief endian 定义
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-03-31
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-03-31<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#ifndef SIMPLEKERNEL_ENDIAN_H
#define SIMPLEKERNEL_ENDIAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "string.h"

static union {
    char          c[4];
    unsigned long mylong;
} endian_test __attribute__((unused)) = {{'l', '?', '?', 'b'}};

// 'l' 为小端，'b' 为大端
#define ENDIANNESS ((char)endian_test.mylong)

#define __LITTLE_ENDIAN 1234
#define __BIG_ENDIAN 4321
#define __PDP_ENDIAN 3412

#if defined(__riscv) || defined(__i386__) || defined(__x86_64__)
#define __BYTE_ORDER __LITTLE_ENDIAN
#else
#define __BYTE_ORDER __BIG_ENDIAN
#endif

#define BIG_ENDIAN __BIG_ENDIAN
#define LITTLE_ENDIAN __LITTLE_ENDIAN
#define PDP_ENDIAN __PDP_ENDIAN
#define BYTE_ORDER __BYTE_ORDER

static inline uint16_t __bswap16(uint16_t __x) {
    return (__x << 8) | (__x >> 8);
}

static inline uint32_t __bswap32(uint32_t __x) {
    return (__x >> 24) | (__x >> 8 & 0xff00) | (__x << 8 & 0xff0000) |
           (__x << 24);
}

static inline uint64_t __bswap64(uint64_t __x) {
    return (__bswap32(__x) + (0ULL << 32)) | __bswap32(__x >> 32);
}

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define htobe16(x) __bswap16(x)
#define be16toh(x) __bswap16(x)
#define htobe32(x) __bswap32(x)
#define be32toh(x) __bswap32(x)
#define htobe64(x) __bswap64(x)
#define be64toh(x) __bswap64(x)
#define htole16(x) (uint16_t)(x)
#define le16toh(x) (uint16_t)(x)
#define htole32(x) (uint32_t)(x)
#define le32toh(x) (uint32_t)(x)
#define htole64(x) (uint64_t)(x)
#define le64toh(x) (uint64_t)(x)
#else
#define htobe16(x) (uint16_t)(x)
#define be16toh(x) (uint16_t)(x)
#define htobe32(x) (uint32_t)(x)
#define be32toh(x) (uint32_t)(x)
#define htobe64(x) (uint64_t)(x)
#define be64toh(x) (uint64_t)(x)
#define htole16(x) __bswap16(x)
#define le16toh(x) __bswap16(x)
#define htole32(x) __bswap32(x)
#define le32toh(x) __bswap32(x)
#define htole64(x) __bswap64(x)
#define le64toh(x) __bswap64(x)
#endif

#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define betoh16(x) __bswap16(x)
#define betoh32(x) __bswap32(x)
#define betoh64(x) __bswap64(x)
#define letoh16(x) (uint16_t)(x)
#define letoh32(x) (uint32_t)(x)
#define letoh64(x) (uint64_t)(x)
#else
#define betoh16(x) (uint16_t)(x)
#define betoh32(x) (uint32_t)(x)
#define betoh64(x) (uint64_t)(x)
#define letoh16(x) __bswap16(x)
#define letoh32(x) __bswap32(x)
#define letoh64(x) __bswap64(x)
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEKERNEL_ENDIAN_H */
