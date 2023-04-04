
/**
 * @file string.c
 * @brief string 定义
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

#ifdef __cplusplus
extern "C" {
#endif

#include "stddef.h"
#include "string.h"
#include "assert.h"

#define __glibc_unlikely(cond) __builtin_expect((cond), 0)
#define __glibc_likely(cond) __builtin_expect((cond), 1)

// 获取字符串长度
size_t strlen(const char *_str) {
    size_t len = 0;
    while (_str[len]) {
        len++;
    }
    return len;
}

size_t strnlen(const char *_str, size_t _maxlen) {
    const char              *char_ptr, *end_ptr = _str + _maxlen;
    const unsigned long int *longword_ptr;
    unsigned long int        longword, himagic, lomagic;

    if (_maxlen == 0) {
        return 0;
    }

    if (__glibc_unlikely(end_ptr < _str)) {
        end_ptr = (const char *)~0UL;
    }

    /* Handle the first few characters by reading one character at a time.
       Do this until CHAR_PTR is aligned on a longword boundary.  */
    for (char_ptr = _str;
         ((unsigned long int)char_ptr & (sizeof(longword) - 1)) != 0;
         ++char_ptr) {
        if (*char_ptr == '\0') {
            if (char_ptr > end_ptr) {
                char_ptr = end_ptr;
            }
            return char_ptr - _str;
        }
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
            if (cp[0] == 0) {
                break;
            }
            char_ptr = cp + 1;
            if (cp[1] == 0) {
                break;
            }
            char_ptr = cp + 2;
            if (cp[2] == 0) {
                break;
            }
            char_ptr = cp + 3;
            if (cp[3] == 0) {
                break;
            }
            if (sizeof(longword) > 4) {
                char_ptr = cp + 4;
                if (cp[4] == 0) {
                    break;
                }
                char_ptr = cp + 5;
                if (cp[5] == 0) {
                    break;
                }
                char_ptr = cp + 6;
                if (cp[6] == 0) {
                    break;
                }
                char_ptr = cp + 7;
                if (cp[7] == 0) {
                    break;
                }
            }
        }
        char_ptr = end_ptr;
    }

    if (char_ptr > end_ptr) {
        char_ptr = end_ptr;
    }
    return char_ptr - _str;
}

// 如果 src > dest, 则返回值大于 0，如果 src = dest, 则返回值等于 0，
// 如果 srd  < dest ,则返回值小于 0。
int strcmp(const char *_src, const char *_dest) {
    while (*_src && *_dest && (*_src == *_dest)) {
        _src++;
        _dest++;
    }
    return *_src - *_dest;
}

int strncmp(const char *_s1, const char *_s2, size_t _n) {

    if (_n == 0) {
        return 0;
    }
    do {
        if (*_s1 != *_s2++) {
            return (*(const unsigned char *)_s1 -
                    *(const unsigned char *)(_s2 - 1));
        }
        if (*_s1++ == '\0') {
            break;
        }
    } while (--_n != 0);
    return 0;
}

char *strcpy(char *_dest, const char *_src) {
    char *address = _dest;
    while ((*_dest++ = *_src++) != '\0') {
        ;
    }
    return address;
}

char *strncpy(char *_s1, const char *_s2, size_t _n) {
    size_t size = strnlen(_s2, _n);
    if (size != _n) {
        memset(_s1 + size, '\0', _n - size);
    }
    return memcpy(_s1, _s2, size);
}

void backspace(char *_src) {
    size_t len    = strlen(_src);
    _src[len - 1] = '\0';
}

void append(char *_src, char _dest) {
    size_t len    = strlen(_src);
    _src[len]     = _dest;
    _src[len + 1] = '\0';
}

char *strcat(char *_dest, const char *_src) {
    uint8_t *add_d = (uint8_t *)_dest;
    if (_dest != 0 && _src != 0) {
        while (*add_d) {
            add_d++;
        }
        while (*_src) {
            *add_d++ = *_src++;
        }
    }
    return _dest;
}

char *strchr(register const char *_s, int _c) {
    do {
        if (*_s == _c) {
            return (char *)_s;
        }
    } while (*_s++);
    return NULL;
}

void *memcpy(void *_dest, const void *_src, size_t _len) {
    char       *d = _dest;
    const char *s = _src;
    while (_len--) {
        *d++ = *s++;
    }
    return _dest;
}

void *memset(void *_dest, int _val, size_t _len) {
    unsigned char *ptr = _dest;
    while (_len-- > 0) {
        *ptr++ = _val;
    }
    return _dest;
}

void bzero(void *_dest, uint32_t _len) {
    memset(_dest, 0, _len);
}

int memcmp(const void *_str1, const void *_str2, size_t _count) {
    register const unsigned char *s1 = (const unsigned char *)_str1;
    register const unsigned char *s2 = (const unsigned char *)_str2;

    while (_count-- > 0) {
        if (*s1++ != *s2++) {
            return s1[-1] < s2[-1] ? -1 : 1;
        }
    }
    return 0;
}

void *memmove(void *_dest, const void *_src, size_t _len) {
    char       *d = _dest;
    const char *s = _src;
    if (d < s) {
        while (_len--) {
            *d++ = *s++;
        }
    }
    else {
        const char *lasts = s + (_len - 1);
        char       *lastd = d + (_len - 1);
        while (_len--) {
            *lastd-- = *lasts--;
        }
    }
    return _dest;
}

void strrev(char *_str) {
    int           i;
    int           j;
    unsigned char a;
    unsigned      len = strlen((const char *)_str);
    for (i = 0, j = len - 1; i < j; i++, j--) {
        a       = _str[i];
        _str[i] = _str[j];
        _str[j] = a;
    }
    return;
}

#ifdef __cplusplus
}
#endif
