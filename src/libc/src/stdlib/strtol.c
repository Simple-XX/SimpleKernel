
/**
 * @file strtol.c
 * @brief strtol 定义
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * Based on libgcc
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

#include "ctype.h"
#include "limits.h"
#include "math.h"
#include "stdlib.h"

/*
 * Convert a string to a long integer.
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */
long strtol(const char* _nptr, char** _endptr, int _base) {
    const char*   s = _nptr;
    unsigned long acc;
    int           c;
    unsigned long cutoff;
    int           neg = 0, any, cutlim;

    /*
     * Skip white space and pick up leading +/- sign if any.
     * If base is 0, allow 0x for hex and 0 for octal, else
     * assume decimal; if base is already 16, allow 0x.
     */
    do {
        c = *s++;
    } while (isspace(c));
    if (c == '-') {
        neg = 1;
        c   = *s++;
    }
    else if (c == '+') {
        c = *s++;
    }
    if ((_base == 0 || _base == 16) && c == '0' && (*s == 'x' || *s == 'X')) {
        c      = s[1];
        s     += 2;
        _base  = 16;
    }
    if (_base == 0) {
        _base = c == '0' ? 8 : 10;
    }

    /*
     * Compute the cutoff value between legal numbers and illegal
     * numbers.  That is the largest legal value, divided by the
     * base.  An input number that is greater than this value, if
     * followed by a legal input character, is too big.  One that
     * is equal to this value may be valid or not; the limit
     * between valid and invalid numbers is then based on the last
     * digit.  For instance, if the range for longs is
     * [-2147483648..2147483647] and the input base is 10,
     * cutoff will be set to 214748364 and cutlim to either
     * 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
     * a value > 214748364, or equal but the next digit is > 7 (or 8),
     * the number is too big, and we will return a range error.
     *
     * Set any if any `digits' consumed; make it negative to indicate
     * overflow.
     */
    cutoff  = neg ? -(unsigned long)LONG_MIN : LONG_MAX;
    cutlim  = cutoff % (unsigned long)_base;
    cutoff /= (unsigned long)_base;
    for (acc = 0, any = 0;; c = *s++) {
        if (isdigit(c)) {
            c -= '0';
        }
        else if (isalpha(c)) {
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        }
        else {
            break;
        }
        if (c >= _base) {
            break;
        }
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim)) {
            any = -1;
        }
        else {
            any  = 1;
            acc *= _base;
            acc += c;
        }
    }
    if (any < 0) {
        acc = neg ? LONG_MIN : LONG_MAX;
    }
    else if (neg) {
        acc = -acc;
    }
    if (_endptr != 0) {
        *_endptr = (char*)(any ? s - 1 : _nptr);
    }
    return (acc);
}

/*
 * Convert a string to a long long integer.
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */
long long strtoll(const char* _nptr, char** _endptr, int _base) {
    const char* s = _nptr;
    long long   acc;
    int         c;
    long long   cutoff;
    int         neg = 0, any, cutlim;
    /*
     * Skip white space and pick up leading +/- sign if any.
     * If base is 0, allow 0x for hex and 0 for octal, else
     * assume decimal; if base is already 16, allow 0x.
     */
    do {
        c = *s++;
    } while (isspace(c));
    if (c == '-') {
        neg = 1;
        c   = *s++;
    }
    else if (c == '+') {
        c = *s++;
    }
    if ((_base == 0 || _base == 16) && c == '0' && (*s == 'x' || *s == 'X')) {
        c      = s[1];
        s     += 2;
        _base  = 16;
    }
    if (_base == 0) {
        _base = c == '0' ? 8 : 10;
    }
    /*
     * Compute the cutoff value between legal numbers and illegal
     * numbers.  That is the largest legal value, divided by the
     * base.  An input number that is greater than this value, if
     * followed by a legal input character, is too big.  One that
     * is equal to this value may be valid or not; the limit
     * between valid and invalid numbers is then based on the last
     * digit.  For instance, if the range for longs is
     * [-2147483648..2147483647] and the input base is 10,
     * cutoff will be set to 214748364 and cutlim to either
     * 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
     * a value > 214748364, or equal but the next digit is > 7 (or 8),
     * the number is too big, and we will return a range error.
     *
     * Set any if any `digits' consumed; make it negative to indicate
     * overflow.
     */
    cutoff = neg ? (unsigned long long)-(LLONG_MIN + LLONG_MAX) + LLONG_MAX
                 : LLONG_MAX;

#if defined(__i386__)
    cutoff = udivmoddi4(cutoff, _base, (unsigned long long*)&cutlim);
#else
    cutlim  = cutoff % (long long)_base;
    cutoff /= (long long)_base;
#endif
    for (acc = 0, any = 0;; c = *s++) {
        if (isdigit(c)) {
            c -= '0';
        }
        else if (isalpha(c)) {
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        }
        else {
            break;
        }
        if (c >= _base) {
            break;
        }
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim)) {
            any = -1;
        }
        else {
            any  = 1;
            acc *= _base;
            acc += c;
        }
    }
    if (any < 0) {
        acc = neg ? LLONG_MIN : LLONG_MAX;
    }
    else if (neg) {
        acc = -acc;
    }
    if (_endptr != 0) {
        *_endptr = (char*)(any ? s - 1 : _nptr);
    }
    return (acc);
}
