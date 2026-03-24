/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// 底层字符输出原语，由各架构的 early_console.cpp 实现
void etl_putchar(int c);

#ifdef __cplusplus
}
#endif
