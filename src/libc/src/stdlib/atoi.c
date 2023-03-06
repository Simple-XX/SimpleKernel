
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// atoi.c for Simple-XX/SimpleKernel.

#include "stddef.h"
#include "stdlib.h"

int abs(int _i) {
    return _i < 0 ? -_i : _i;
}

int atoi(const char *_str) {
    return (int)strtol(_str, (char **)NULL, 10);
}

long atol(const char *_str) {
    return (long)strtoll(_str, (char **)NULL, 10);
}

long long atoll(const char *_str) {
    return (long long)strtoll(_str, (char **)NULL, 10);
}
