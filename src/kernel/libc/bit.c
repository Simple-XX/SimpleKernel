
/**
 * @file bit.c
 * @brief 位操作相关函数
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

#include <stdint.h>

// These functions return the number of leading 0-bits in a, starting at the
// most significant bit position.  If a is zero, the result is undefined.
int __clzsi2(unsigned int a) {
  if (a == 0) {
    return 32;
  }
  int count = 0;
  while ((a & 0x80000000) == 0) {
    a <<= 1;
    count++;
  }
  return count;
}

int __clzdi2(unsigned long a) {
  if (a == 0) {
    return 64;
  }
  int count = 0;
  while ((a & 0x8000000000000000UL) == 0) {
    a <<= 1;
    count++;
  }
  return count;
}

int __clzti2(unsigned long long a) {
  if (a == 0) {
    return 64;
  }
  int count = 0;
  while ((a & 0x8000000000000000ULL) == 0) {
    a <<= 1;
    count++;
  }
  return count;
}

// These functions return the number of trailing 0-bits in a, starting at the
// least significant bit position.  If a is zero, the result is undefined.
int __ctzsi2(unsigned int a) {
  if (a == 0) {
    return 32;  // Return 32 if no bits are set
  }
  int count = 0;
  while ((a & 1) == 0) {
    a >>= 1;
    count++;
  }
  return count;
}

int __ctzdi2(unsigned long a) {
  if (a == 0) {
    return 64;
  }
  int count = 0;
  while ((a & 1) == 0) {
    a >>= 1;
    count++;
  }
  return count;
}

int __ctzti2(unsigned long long a) {
  if (a == 0) {
    return 64;
  }
  int count = 0;
  while ((a & 1) == 0) {
    a >>= 1;
    count++;
  }
  return count;
}

// These functions return the index of the least significant 1-bit in a, or the
// value zero if a is zero.  The least significant bit is index one.
int __ffsdi2(unsigned long a) {
  if (a == 0) {
    return 0;
  }
  int index = 1;
  while ((a & 1) == 0) {
    a >>= 1;
    index++;
  }
  return index;
}

int __ffsti2(unsigned long long a) {
  if (a == 0) {
    return 0;
  }
  int index = 1;
  while ((a & 1) == 0) {
    a >>= 1;
    index++;
  }
  return index;
}

// These functions return the value zero if the number of bits set in a is even,
// and the value one otherwise.
int __paritysi2(unsigned int a) {
  a ^= a >> 16;
  a ^= a >> 8;
  a ^= a >> 4;
  a ^= a >> 2;
  a ^= a >> 1;
  return a & 1;
}

int __paritydi2(unsigned long a) {
  a ^= a >> 32;
  a ^= a >> 16;
  a ^= a >> 8;
  a ^= a >> 4;
  a ^= a >> 2;
  a ^= a >> 1;
  return a & 1;
}

int __parityti2(unsigned long long a) {
  a ^= a >> 32;
  a ^= a >> 16;
  a ^= a >> 8;
  a ^= a >> 4;
  a ^= a >> 2;
  a ^= a >> 1;
  return a & 1;
}

// These functions return the number of bits set in a.
int __popcountsi2(unsigned int a) {
  int count = 0;
  while (a) {
    a &= a - 1;
    count++;
  }
  return count;
}

int __popcountdi2(unsigned long a) {
  int count = 0;
  while (a) {
    a &= a - 1;
    count++;
  }
  return count;
}

int __popcountti2(unsigned long long a) {
  int count = 0;
  while (a) {
    a &= a - 1;
    count++;
  }
  return count;
}

// These functions return the a byteswapped.
int32_t __bswapsi2(int32_t a) {
  return (((a & 0xFF000000) >> 24) | ((a & 0x00FF0000) >> 8) |
          ((a & 0x0000FF00) << 8) | ((a & 0x000000FF) << 24));
}

int64_t __bswapdi2(int64_t a) {
  return (
      ((a & 0xFF00000000000000ULL) >> 56) |
      ((a & 0x00FF000000000000ULL) >> 40) |
      ((a & 0x0000FF0000000000ULL) >> 24) | ((a & 0x000000FF00000000ULL) >> 8) |
      ((a & 0x00000000FF000000ULL) << 8) | ((a & 0x0000000000FF0000ULL) << 24) |
      ((a & 0x000000000000FF00ULL) << 40) |
      ((a & 0x00000000000000FFULL) << 56));
}
