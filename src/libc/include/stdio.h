
/**
 * @file stdio.h
 * @brief stdio 定义
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * Based on https://github.com/mpaland/printf
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

#ifndef SIMPLEKERNEL_STDIO_H
#define SIMPLEKERNEL_STDIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdarg.h"
#include "stddef.h"
#include "stdint.h"

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
int sprintf_(char* _buffer, const char* _format, ...);

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
#define snprintf  snprintf_
#define vsnprintf vsnprintf_
int snprintf_(char* _buffer, size_t _count, const char* _format, ...);
int vsnprintf_(char* _buffer, size_t _count, const char* _format, va_list _va);
int _vsnprintf(char* _buffer, const size_t _maxlen, const char* _format,
               va_list _va);

int32_t printf(const char* _fmt, ...);

int32_t info(const char* _fmt, ...);

int32_t warn(const char* _fmt, ...);

int32_t err(const char* _fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEKERNEL_STDIO_H */
