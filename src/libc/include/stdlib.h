
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

int       abs(int);
int       atoi(const char *);
long      atol(const char *);
long long atoll(const char *);
int       itoa(int num, char *str, int len, int base);
long      strtol(const char *nptr, char **endptr, int base);
long long strtoll(const char *nptr, char **endptr, int base);

#ifdef __cplusplus
}
#endif

#endif /* _STDLIB_H_ */
