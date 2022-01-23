
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// stdlib.h for Simple-XX/SimpleKernel.

#ifndef _STDLIB_H_
#define _STDLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stddef.h"

int       abs(int);
int       atoi(const char *);
long      atol(const char *);
long long atoll(const char *);
int       itoa(int num, char *str, int len, int base);
long      strtol(const char *nptr, char **endptr, int base);
long long strtoll(const char *nptr, char **endptr, int base);

void *malloc(size_t size);

void free(void *ptr);

void *kmalloc(size_t size);

void kfree(void *ptr);

#ifdef __cplusplus
}
#endif

#endif /* _STDLIB_H_ */
