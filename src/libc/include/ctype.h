
/**
 * @file ctype.h
 * @brief ctype 定义
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

#ifndef SIMPLEKERNEL_CTYPE_H
#define SIMPLEKERNEL_CTYPE_H

#ifdef __cplusplus
extern "C" {
#endif

#define isupper(_c) (_c >= 'A' && _c <= 'Z')
#define isalpha(_c) ((_c >= 'A' && _c <= 'Z') || (_c >= 'a' && _c <= 'z'))
#define isspace(_c) (_c == ' ' || _c == '\t' || _c == '\n' || _c == '\12')
#define isdigit(_c) (_c >= '0' && _c <= '9')

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEKERNEL_CTYPE_H */
