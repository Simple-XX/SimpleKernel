/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "sk_string.h"

#ifdef __cplusplus
extern "C" {
#endif

// 复制内存块
void* memcpy(void* dest, const void* src, size_t n) {
  char* d = (char*)dest;
  const char* s = (const char*)src;
  while (n--) {
    *d++ = *s++;
  }
  return dest;
}

// 复制内存块，可以处理重叠区域。
void* memmove(void* dest, const void* src, size_t n) {
  char* d = (char*)dest;
  const char* s = (const char*)src;
  if (d < s) {
    while (n--) {
      *d++ = *s++;
    }
  } else {
    const char* lasts = s + (n - 1);
    char* lastd = d + (n - 1);
    while (n--) {
      *lastd-- = *lasts--;
    }
  }
  return dest;
}

// 设置内存块
void* memset(void* dest, int val, size_t n) {
  unsigned char* ptr = (unsigned char*)dest;
  while (n-- > 0) {
    *ptr++ = val;
  }
  return dest;
}

// 比较内存块
int memcmp(const void* str1, const void* str2, size_t n) {
  const unsigned char* s1 = (const unsigned char*)str1;
  const unsigned char* s2 = (const unsigned char*)str2;

  while (n-- > 0) {
    if (*s1++ != *s2++) {
      return s1[-1] < s2[-1] ? -1 : 1;
    }
  }
  return 0;
}

// 在内存块中查找字符
const void* memchr(const void* str, int c, size_t n) {
  const unsigned char* src = (const unsigned char*)str;
  unsigned char uc = (unsigned char)c;

  while (n-- > 0) {
    if (*src == uc) {
      return (void*)src;
    }
    src++;
  }
  return NULL;
}

// 复制字符串
char* strcpy(char* dest, const char* src) {
  char* address = dest;
  while ((*dest++ = *src++) != '\0') {
    ;
  }
  return address;
}

// 复制指定长度的字符串
char* strncpy(char* dest, const char* src, size_t n) {
  size_t size = strnlen(src, n);
  if (size != n) {
    memset(dest + size, '\0', n - size);
  }
  return (char*)memcpy(dest, src, size);
}

// 连接字符串
char* strcat(char* dest, const char* src) {
  char* add_d = dest;
  if (dest != 0 && src != 0) {
    while (*add_d) {
      add_d++;
    }
    while (*src) {
      *add_d++ = *src++;
    }
    *add_d = '\0';
  }
  return dest;
}

// 比较字符串
int strcmp(const char* s1, const char* s2) {
  while (*s1 && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

// 比较指定长度的字符串
int strncmp(const char* s1, const char* s2, size_t n) {
  if (n == 0) {
    return 0;
  }
  do {
    if (*s1 != *s2++) {
      return (*(const unsigned char*)s1 - *(const unsigned char*)(s2 - 1));
    }
    if (*s1++ == '\0') {
      break;
    }
  } while (--n != 0);
  return 0;
}

// 获取字符串长度
size_t strlen(const char* s) {
  size_t len = 0;
  while (s[len]) {
    len++;
  }
  return len;
}

// 获取指定字符串长度
size_t strnlen(const char* s, size_t n) {
  const char* p = s;
  while (n-- > 0 && *p) {
    p++;
  }
  return p - s;
}

// 查找字符在字符串中的首次出现
const char* strchr(const char* s, int c) {
  char ch = (char)c;
  do {
    if (*s == ch) {
      return (char*)s;
    }
  } while (*s++);
  return NULL;
}

// 反向查找字符在字符串中的首次出现
const char* strrchr(const char* s, int c) {
  char* rtnval = 0;
  char ch = (char)c;

  do {
    if (*s == ch) {
      rtnval = (char*)s;
    }
  } while (*s++);
  return (rtnval);
}

#ifdef __cplusplus
}
#endif
