/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void* malloc(size_t size);
void free(void* ptr);
void* calloc(size_t num, size_t size);
void* realloc(void* ptr, size_t new_size);
void* aligned_alloc(size_t alignment, size_t size);
void aligned_free(void* ptr);

/**
 * @brief 将字符串转换为双精度浮点数
 * @param nptr 指向要转换的字符串的指针
 * @return 转换后的双精度浮点数
 */
double atof(const char* nptr);

/**
 * @brief 将字符串转换为整数
 * @param nptr 指向要转换的字符串的指针
 * @return 转换后的整数值
 */
int atoi(const char* nptr);

/**
 * @brief 将字符串转换为长整数
 * @param nptr 指向要转换的字符串的指针
 * @return 转换后的长整数值
 */
long int atol(const char* nptr);

/**
 * @brief 将字符串转换为长长整数
 * @param nptr 指向要转换的字符串的指针
 * @return 转换后的长长整数值
 */
long long int atoll(const char* nptr);

/**
 * @brief 将字符串转换为双精度浮点数，并可以获取转换结束位置
 * @param nptr 指向要转换的字符串的指针
 * @param endptr 指向指针的指针，用于存储转换结束位置
 * @return 转换后的双精度浮点数
 */
double strtod(const char* nptr, char** endptr);

#if (defined(__aarch64__) && defined(__ARM_FP)) || defined(__riscv)
/**
 * @brief 将字符串转换为单精度浮点数，并可以获取转换结束位置
 * @param nptr 指向要转换的字符串的指针
 * @param endptr 指向指针的指针，用于存储转换结束位置
 * @return 转换后的单精度浮点数
 */
float strtof(const char* nptr, char** endptr);

/**
 * @brief 将字符串转换为长双精度浮点数，并可以获取转换结束位置
 * @param nptr 指向要转换的字符串的指针
 * @param endptr 指向指针的指针，用于存储转换结束位置
 * @return 转换后的长双精度浮点数
 */
long double strtold(const char* nptr, char** endptr);
#endif
/**
 * @brief 将字符串按指定进制转换为长整数，并可以获取转换结束位置
 * @param nptr 指向要转换的字符串的指针
 * @param endptr 指向指针的指针，用于存储转换结束位置
 * @param base 转换的进制（2-36，0表示自动检测）
 * @return 转换后的长整数值
 */
long int strtol(const char* nptr, char** endptr, int base);

/**
 * @brief 将字符串按指定进制转换为长长整数，并可以获取转换结束位置
 * @param nptr 指向要转换的字符串的指针
 * @param endptr 指向指针的指针，用于存储转换结束位置
 * @param base 转换的进制（2-36，0表示自动检测）
 * @return 转换后的长长整数值
 */
long long int strtoll(const char* nptr, char** endptr, int base);

/**
 * @brief 将字符串按指定进制转换为无符号长整数，并可以获取转换结束位置
 * @param nptr 指向要转换的字符串的指针
 * @param endptr 指向指针的指针，用于存储转换结束位置
 * @param base 转换的进制（2-36，0表示自动检测）
 * @return 转换后的无符号长整数值
 */
unsigned long int strtoul(const char* nptr, char** endptr, int base);

/**
 * @brief 将字符串按指定进制转换为无符号长长整数，并可以获取转换结束位置
 * @param nptr 指向要转换的字符串的指针
 * @param endptr 指向指针的指针，用于存储转换结束位置
 * @param base 转换的进制（2-36，0表示自动检测）
 * @return 转换后的无符号长长整数值
 */
unsigned long long int strtoull(const char* nptr, char** endptr, int base);

#ifdef __cplusplus
}
#endif
