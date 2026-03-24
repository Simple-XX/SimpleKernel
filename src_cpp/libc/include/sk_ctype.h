/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// 检查字符是否为字母或数字
int isalnum(int c);
// 检查字符是否为字母
int isalpha(int c);
// 检查字符是否为空白字符（空格或制表符）
int isblank(int c);
// 检查字符是否为控制字符
int iscntrl(int c);
// 检查字符是否为十进制数字（0-9）
int isdigit(int c);
// 检查字符是否为可打印字符（不包括空格）
int isgraph(int c);
// 检查字符是否为小写字母
int islower(int c);
// 检查字符是否为可打印字符（包括空格）
int isprint(int c);
// 检查字符是否为标点符号
int ispunct(int c);
// 检查字符是否为空白字符（空格、制表符、换行符等）
int isspace(int c);
// 检查字符是否为大写字母
int isupper(int c);
// 检查字符是否为十六进制数字（0-9、a-f、A-F）
int isxdigit(int c);
// 将字符转换为小写
int tolower(int c);
// 将字符转换为大写
int toupper(int c);

#ifdef __cplusplus
}
#endif
