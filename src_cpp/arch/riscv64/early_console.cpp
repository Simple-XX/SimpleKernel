/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <opensbi_interface.h>

/// 早期控制台输出单字符，通过 SBI 调用实现
extern "C" auto etl_putchar(int c) -> void { sbi_debug_console_write_byte(c); }
