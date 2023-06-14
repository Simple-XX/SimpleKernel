/*
 * string.c
 *
 * Copyright (C) 2021 bzt (bztsrc@gitlab)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This file is part of the POSIX-UEFI package.
 * @brief Implementing functions which are defined in string.h
 *
 */

#include <uefi.h>

void *memcpy(void *dst, const void *src, size_t n)
{
    uint8_t *a=(uint8_t*)dst,*b=(uint8_t*)src;
    if(src && dst && src != dst && n>0) {
        while(n--) *a++ = *b++;
    }
    return dst;
}

void *memmove(void *dst, const void *src, size_t n)
{
    uint8_t *a=(uint8_t*)dst,*b=(uint8_t*)src;
    if(src && dst && src != dst && n>0) {
        if(a>b && a<b+n) {
            a+=n-1; b+=n-1; while(n-->0) *a--=*b--;
        } else {
            while(n--) *a++ = *b++;
        }
    }
    return dst;
}

void *memset(void *s, int c, size_t n)
{
    uint8_t *p=(uint8_t*)s;
    if(s && n>0) {
        while(n--) *p++ = (uint8_t)c;
    }
    return s;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    uint8_t *a=(uint8_t*)s1,*b=(uint8_t*)s2;
    if(s1 && s2 && s1 != s2 && n>0) {
        while(n--) {
            if(*a != *b) return *a - *b;
            a++; b++;
        }
    }
    return 0;
}

void *memchr(const void *s, int c, size_t n)
{
    uint8_t *e, *p=(uint8_t*)s;
    if(s && n>0) {
        for(e=p+n; p<e; p++) if(*p==(uint8_t)c) return p;
    }
    return NULL;
}

void *memrchr(const void *s, int c, size_t n)
{
    uint8_t *e, *p=(uint8_t*)s;
    if(s && n>0) {
        for(e=p+n; p<e; --e) if(*e==(uint8_t)c) return e;
    }
    return NULL;
}

void *memmem(const void *haystack, size_t hl, const void *needle, size_t nl)
{
    uint8_t *c = (uint8_t*)haystack;
    if(!haystack || !needle || !hl || !nl || nl > hl) return NULL;
    hl -= nl - 1;
    while(hl) {
        if(!memcmp(c, needle, nl)) return c;
        c++; hl--;
    }
    return NULL;
}

void *memrmem(const void *haystack, size_t hl, const void *needle, size_t nl)
{
    uint8_t *c = (uint8_t*)haystack;
    if(!haystack || !needle || !hl || !nl || nl > hl) return NULL;
    hl -= nl;
    c += hl;
    while(hl) {
        if(!memcmp(c, needle, nl)) return c;
        c--; hl--;
    }
    return NULL;
}

char_t *strcpy(char_t *dst, const char_t *src)
{
    char_t *s = dst;
    if(src && dst && src != dst) {
        while(*src) {*dst++=*src++;} *dst=0;
    }
    return s;
}

char_t *strncpy(char_t *dst, const char_t *src, size_t n)
{
    char_t *s = dst;
    const char_t *e = src+n;
    if(src && dst && src != dst && n>0) {
        while(*src && src<e) {*dst++=*src++;} *dst=0;
    }
    return s;
}

char_t *strcat(char_t *dst, const char_t *src)
{
    char_t *s = dst;
    if(src && dst) {
        dst += strlen(dst);
        while(*src) {*dst++=*src++;} *dst=0;
    }
    return s;
}

int strcmp(const char_t *s1, const char_t *s2)
{
    if(s1 && s2 && s1!=s2) {
        while(*s1 && *s1==*s2){s1++;s2++;}
        return *s1-*s2;
    }
    return 0;
}

char_t *strncat(char_t *dst, const char_t *src, size_t n)
{
    char_t *s = dst;
    const char_t *e = src+n;
    if(src && dst && n>0) {
        dst += strlen(dst);
        while(*src && src<e) {*dst++=*src++;} *dst=0;
    }
    return s;
}

int strncmp(const char_t *s1, const char_t *s2, size_t n)
{
    const char_t *e = s1+n;
    if(s1 && s2 && s1!=s2 && n>0) {
        while(s1<e && *s1 && *s1==*s2){s1++;s2++;}
        return *s1-*s2;
    }
    return 0;
}

char_t *strdup(const char_t *s)
{
    size_t i = (strlen(s)+1) * sizeof(char_t);
    char_t *s2 = (char_t *)malloc(i);
    if(s2 != NULL) memcpy(s2, (const void*)s, i);
    return s2;
}

char_t *strchr(const char_t *s, int c)
{
    if(s) {
        while(*s) {
            if(*s == (char_t)c) return (char_t*)s;
            s++;
        }
    }
    return NULL;
}

char_t *strrchr(const char_t *s, int c)
{
    char_t *e;
    if(s) {
        e = (char_t*)s + strlen(s) - 1;
        while(s <= e) {
            if(*e == (char_t)c) return e;
            e--;
        }
    }
    return NULL;
}

char_t *strstr(const char_t *haystack, const char_t *needle)
{
    return memmem(haystack, strlen(haystack) * sizeof(char_t), needle, strlen(needle) * sizeof(char_t));
}

static char_t *_strtok_r(char_t *s, const char_t *d, char_t **p)
{
    int c, sc;
    char_t *tok, *sp;

    if(d == NULL || (s == NULL && (s=*p) == NULL)) return NULL;
again:
    c = *s++;
    for(sp = (char_t *)d; (sc=*sp++)!=0;) {
        if(c == sc) goto again;
    }

    if (c == 0) { *p=NULL; return NULL; }
    tok = s-1;
    while(1) {
        c = *s++;
        sp = (char_t *)d;
        do {
            if((sc=*sp++) == c) {
                if(c == 0) s = NULL;
                else *(s-1) = 0;
                *p = s;
                return tok;
            }
        } while(sc != 0);
    }
    return NULL;
}

char_t *strtok(char_t *s, const char_t *delim)
{
    static char_t *p;
    return _strtok_r (s, delim, &p);
}

char_t *strtok_r(char_t *s, const char_t *delim, char_t **ptr)
{
    return _strtok_r (s, delim, ptr);
}

size_t strlen (const char_t *__s)
{
    size_t ret;

    if(!__s) return 0;
    for(ret = 0; __s[ret]; ret++);
    return ret;
}
