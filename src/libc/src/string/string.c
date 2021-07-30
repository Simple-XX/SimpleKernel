
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// string.c for Simple-XX/SimpleKernel.

#ifdef __cplusplus
extern "C" {
#endif

#include "stddef.h"
#include "string.h"
#include "assert.h"

#define __glibc_unlikely(cond) __builtin_expect((cond), 0)
#define __glibc_likely(cond) __builtin_expect((cond), 1)

// 获取字符串长度
size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}

size_t strnlen(const char *str, size_t maxlen) {
    const char *             char_ptr, *end_ptr = str + maxlen;
    const unsigned long int *longword_ptr;
    unsigned long int        longword, himagic, lomagic;

    if (maxlen == 0)
        return 0;

    if (__glibc_unlikely(end_ptr < str))
        end_ptr = (const char *)~0UL;

    /* Handle the first few characters by reading one character at a time.
       Do this until CHAR_PTR is aligned on a longword boundary.  */
    for (char_ptr = str;
         ((unsigned long int)char_ptr & (sizeof(longword) - 1)) != 0;
         ++char_ptr)
        if (*char_ptr == '\0') {
            if (char_ptr > end_ptr)
                char_ptr = end_ptr;
            return char_ptr - str;
        }

    /* All these elucidatory comments refer to 4-byte longwords,
       but the theory applies equally well to 8-byte longwords.  */

    longword_ptr = (unsigned long int *)char_ptr;

    /* Bits 31, 24, 16, and 8 of this number are zero.  Call these bits
       the "holes."  Note that there is a hole just to the left of
       each byte, with an extra at the end:

       bits:  01111110 11111110 11111110 11111111
       bytes: AAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD

       The 1-bits make sure that carries propagate to the next 0-bit.
       The 0-bits provide holes for carries to fall into.  */
    himagic = 0x80808080L;
    lomagic = 0x01010101L;
    if (sizeof(longword) > 4) {
        /* 64-bit version of the magic.  */
        /* Do the shift in two steps to avoid a warning if long has 32 bits.  */
        himagic = ((himagic << 16) << 16) | himagic;
        lomagic = ((lomagic << 16) << 16) | lomagic;
    }
    assert(sizeof(longword) <= 8);
    // if (sizeof(longword) > 8)
    //     abort();
    assert(sizeof(longword) <= 8);

    /* Instead of the traditional loop which tests each character,
       we will test a longword at a time.  The tricky part is testing
       if *any of the four* bytes in the longword in question are zero.  */
    while (longword_ptr < (unsigned long int *)end_ptr) {
        longword = *longword_ptr++;

        if ((longword - lomagic) & himagic) {
            /* Which of the bytes was the zero?  If none of them were, it was
               a misfire; continue the search.  */

            const char *cp = (const char *)(longword_ptr - 1);

            char_ptr = cp;
            if (cp[0] == 0)
                break;
            char_ptr = cp + 1;
            if (cp[1] == 0)
                break;
            char_ptr = cp + 2;
            if (cp[2] == 0)
                break;
            char_ptr = cp + 3;
            if (cp[3] == 0)
                break;
            if (sizeof(longword) > 4) {
                char_ptr = cp + 4;
                if (cp[4] == 0)
                    break;
                char_ptr = cp + 5;
                if (cp[5] == 0)
                    break;
                char_ptr = cp + 6;
                if (cp[6] == 0)
                    break;
                char_ptr = cp + 7;
                if (cp[7] == 0)
                    break;
            }
        }
        char_ptr = end_ptr;
    }

    if (char_ptr > end_ptr)
        char_ptr = end_ptr;
    return char_ptr - str;
}

// 如果 src > dest, 则返回值大于 0，如果 src = dest, 则返回值等于 0，
// 如果 srd  < dest ,则返回值小于 0。
int strcmp(const char *src, const char *dest) {
    while (*src && *dest && (*src == *dest)) {
        src++;
        dest++;
    }
    return *src - *dest;
}

int strncmp(const char *s1, const char *s2, size_t n) {

    if (n == 0)
        return 0;
    do {
        if (*s1 != *s2++)
            return (*(const unsigned char *)s1 -
                    *(const unsigned char *)(s2 - 1));
        if (*s1++ == '\0')
            break;
    } while (--n != 0);
    return 0;
}

char *strcpy(char *dest, const char *src) {
    char *address = dest;
    while ((*dest++ = *src++) != '\0') {
        ;
    }
    return address;
}

char *strncpy(char *s1, const char *s2, size_t n) {
    size_t size = strnlen(s2, n);
    if (size != n)
        memset(s1 + size, '\0', n - size);
    return memcpy(s1, s2, size);
}

void backspace(char *src) {
    size_t len   = strlen(src);
    src[len - 1] = '\0';
}

void append(char *src, char dest) {
    size_t len   = strlen(src);
    src[len]     = dest;
    src[len + 1] = '\0';
}

char *strcat(char *dest, const char *src) {
    uint8_t *add_d = (uint8_t *)dest;
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

char *strchr(register const char *s, int c) {
    do {
        if (*s == c) {
            return (char *)s;
        }
    } while (*s++);
    return (0);
}

void *memcpy(void *dest, const void *src, size_t len) {
    char *      d = dest;
    const char *s = src;
    while (len--)
        *d++ = *s++;
    return dest;
}

void *memset(void *dest, int val, size_t len) {
    unsigned char *ptr = dest;
    while (len-- > 0)
        *ptr++ = val;
    return dest;
}

void bzero(void *dest, uint32_t len) {
    memset(dest, 0, len);
}

int memcmp(const void *str1, const void *str2, size_t count) {
    register const unsigned char *s1 = (const unsigned char *)str1;
    register const unsigned char *s2 = (const unsigned char *)str2;

    while (count-- > 0) {
        if (*s1++ != *s2++)
            return s1[-1] < s2[-1] ? -1 : 1;
    }
    return 0;
}

void *memmove(void *dest, const void *src, size_t len) {
    char *      d = dest;
    const char *s = src;
    if (d < s)
        while (len--)
            *d++ = *s++;
    else {
        const char *lasts = s + (len - 1);
        char *      lastd = d + (len - 1);
        while (len--)
            *lastd-- = *lasts--;
    }
    return dest;
}

void strrev(char *str) {
    int           i;
    int           j;
    unsigned char a;
    unsigned      len = strlen((const char *)str);
    for (i = 0, j = len - 1; i < j; i++, j--) {
        a      = str[i];
        str[i] = str[j];
        str[j] = a;
    }
}

#ifdef __cplusplus
}
#endif
