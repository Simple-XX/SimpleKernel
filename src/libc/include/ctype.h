
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

#define isupper(c) (c >= 'A' && c <= 'Z')
#define isalpha(c) ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
#define isspace(c) (c == ' ' || c == '\t' || c == '\n' || c == '\12')
#define isdigit(c) (c >= '0' && c <= '9')

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEKERNEL_CTYPE_H */
