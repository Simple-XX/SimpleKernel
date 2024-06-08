
/**
 * @file string.c
 * @brief string 实现
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-05-08
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-05-08<td>Zone.N<td>创建文件
 * </table>
 */

#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif

// 复制内存块
void *memcpy(void *dest, const void *src, size_t n) {
  char *d = dest;
  const char *s = src;
  while (n--) {
    *d++ = *s++;
  }
  return dest;
}

// 复制内存块，可以处理重叠区域。
void *memmove(void *dest, const void *src, size_t n) {
  char *d = dest;
  const char *s = src;
  if (d < s) {
    while (n--) {
      *d++ = *s++;
    }
  } else {
    const char *lasts = s + (n - 1);
    char *lastd = d + (n - 1);
    while (n--) {
      *lastd-- = *lasts--;
    }
  }
  return dest;
}

// 设置内存块
void *memset(void *dest, int val, size_t n) {
  unsigned char *ptr = dest;
  while (n-- > 0) {
    *ptr++ = val;
  }
  return dest;
}

// 比较内存块
int memcmp(const void *str1, const void *str2, size_t n) {
  register const unsigned char *s1 = (const unsigned char *)str1;
  register const unsigned char *s2 = (const unsigned char *)str2;

  while (n-- > 0) {
    if (*s1++ != *s2++) {
      return s1[-1] < s2[-1] ? -1 : 1;
    }
  }
  return 0;
}

// 复制字符串
char *strcpy(char *dest, const char *src) {
  char *address = dest;
  while ((*dest++ = *src++) != '\0') {
    ;
  }
  return address;
}

// 复制指定长度的字符串
char *strncpy(char *dest, const char *src, size_t n) {
  size_t size = strnlen(src, n);
  if (size != n) {
    memset(dest + size, '\0', n - size);
  }
  return memcpy(dest, src, size);
}

// 连接字符串
char *strcat(char *dest, const char *src) {
  char *add_d = dest;
  if (dest != 0 && src != 0) {
    while (*add_d) {
      add_d++;
    }
    while (*src) {
      *add_d++ = *src++;
    }
  }
  return dest;
}

// 比较字符串
int strcmp(const char *s1, const char *s2) {
  while (*s2 && *s1 && (*s2 == *s1)) {
    s2++;
    s1++;
  }
  return *s2 - *s1;
}

// 比较指定长度的字符串
int strncmp(const char *s1, const char *s2, size_t n) {
  if (n == 0) {
    return 0;
  }
  do {
    if (*s1 != *s2++) {
      return (*(const unsigned char *)s1 - *(const unsigned char *)(s2 - 1));
    }
    if (*s1++ == '\0') {
      break;
    }
  } while (--n != 0);
  return 0;
}

// 获取字符串长度
size_t strlen(const char *s) {
  size_t len = 0;
  while (s[len]) {
    len++;
  }
  return len;
}

// 获取指定字符串长度
size_t strnlen(const char *s, size_t n) {
  const char *p = s;
  while (n-- > 0 && *p) {
    p++;
  }
  return p - s;
}

// 查找字符在字符串中的首次出现
char *strchr(const char *s, int c) {
  do {
    if (*s == c) {
      return (char *)s;
    }
  } while (*s++);
  return NULL;
}

#ifdef __cplusplus
}
#endif
