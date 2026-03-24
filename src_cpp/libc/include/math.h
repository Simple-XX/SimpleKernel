/**
 * @copyright Copyright The SimpleKernel Contributors
 */

/// @todo 等待 c++26 对 math 的 freestanding 支持后删除此文件
#pragma GCC system_header

#ifdef __cplusplus
#define _GLIBCXX_INCLUDE_NEXT_C_HEADERS
#endif

#include_next <math.h>  // NOLINT: intentional include_next for wrapper bypass

#ifdef __cplusplus
#undef _GLIBCXX_INCLUDE_NEXT_C_HEADERS
#endif
