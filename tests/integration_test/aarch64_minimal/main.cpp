/**
 * @copyright Copyright The SimpleKernel Contributors
 * @brief AArch64 minimal boot test
 */

#include <cstdarg>
#include <cstdint>

extern "C" {
volatile uint8_t* uart = (uint8_t*)0x9000000;
void putc(char character) { *uart = character; }

struct param {
  uint64_t a;
  uint64_t b;
  uint64_t c;
  uint64_t d;
  uint64_t e;
};

void callee([[maybe_unused]] param var) { putc('e'); }

void main() {
  putc('0');
  callee({});
  putc('1');
}
}
