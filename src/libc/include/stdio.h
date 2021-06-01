
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on https://github.com/mpaland/printf
// stdio.h for Simple-XX/SimpleKernel.

#ifndef _STDIO_H_
#define _STDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stddef.h"
#include "stdarg.h"

/**
 * Tiny sprintf implementation
 * Due to security reasons (buffer overflow) YOU SHOULD CONSIDER USING
 * (V)SNPRINTF INSTEAD! \param buffer A pointer to the buffer where to store the
 * formatted string. MUST be big enough to store the output! \param format A
 * string that specifies the format of the output \return The number of
 * characters that are WRITTEN into the buffer, not counting the terminating
 * null character
 */
#define sprintf sprintf_
int sprintf_(char *buffer, const char *format, ...);

/**
 * Tiny snprintf/vsnprintf implementation
 * \param buffer A pointer to the buffer where to store the formatted string
 * \param count The maximum number of characters to store in the buffer,
 * including a terminating null character \param format A string that specifies
 * the format of the output \param va A value identifying a variable arguments
 * list \return The number of characters that COULD have been written into the
 * buffer, not counting the terminating null character. A value equal or larger
 * than count indicates truncation. Only when the returned value is non-negative
 * and less than count, the string has been completely written.
 */
#define snprintf snprintf_
#define vsnprintf vsnprintf_
int snprintf_(char *buffer, size_t count, const char *format, ...);
int vsnprintf_(char *buffer, size_t count, const char *format, va_list va);
int _vsnprintf(char *buffer, const size_t maxlen, const char *format,
               va_list va);

int printf(const char *fmt, ...);

int info(const char *fmt, ...);

int warn(const char *fmt, ...);

int err(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* _STDIO_H_ */
