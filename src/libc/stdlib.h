
// This file is a part of MRNIU/SimpleKernel (https://github.com/MRNIU/SimpleKernel).
//
// stdlib.h for MRNIU/SimpleKernel.

#ifndef _STDLIB_H_
#define _STDLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

int abs(int);
int atoi(const char *);
long atol(const char *);
long long atoll(const char *);

void * malloc(size_t);
void free(void *);

#ifdef __cplusplus
}
#endif

#endif /* _STDLIB_H_ */
