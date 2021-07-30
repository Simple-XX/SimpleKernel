
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// itoa.c for Simple-XX/SimpleKernel.

#include "stdlib.h"
#include "string.h"

int itoa(int num, char *str, int len, int base) {
    int sum = num;
    int i   = 0;
    int digit;
    if (len == 0) {
        return -1;
    }
    do {
        digit = sum % base;
        if (digit < 0xA) {
            str[i++] = '0' + digit;
        }
        else {
            str[i++] = 'A' + digit - 0xA;
        }
        sum /= base;
    } while (sum && (i < (len - 1)));
    if (i == (len - 1) && sum) {
        return -1;
    }
    str[i] = '\0';
    strrev(str);
    return 0;
}
