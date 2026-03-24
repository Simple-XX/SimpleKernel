/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "sk_stdlib.h"

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "sk_ctype.h"

#ifdef __cplusplus
extern "C" {
#endif

/// 栈保护
uint64_t __stack_chk_guard = 0x595E9FBD94FDA766;

/// 栈保护检查失败后进入死循环
[[noreturn]] void __stack_chk_fail() { while (true); }

// Internal helper for string to number conversion
// Parses magnitude into unsigned long long.
// handles base detection, whitespace, signs.
static unsigned long long strtox_main(const char* nptr, char** endptr, int base,
                                      int* sign_out, int* overflow) {
  const char* s = nptr;
  unsigned long long acc = 0;
  int c;
  unsigned long long cutoff;
  int cutlim;
  int any = 0;
  int negative = 0;

  *overflow = 0;

  // Skip whitespace
  while (isspace(*s)) s++;

  // Check sign
  if (*s == '-') {
    negative = 1;
    s++;
  } else if (*s == '+') {
    s++;
  }
  if (sign_out) *sign_out = negative;

  // Detect base
  if ((base == 0 || base == 16) && *s == '0' && (s[1] == 'x' || s[1] == 'X')) {
    // Hex prefix
    // We speculatively consume it.
    if (isxdigit(s[2])) {
      s += 2;
      base = 16;
    } else {
      // '0x' followed by non-hex.
      // if base==0, it's octal 0.
      if (base == 0) base = 8;
    }
  }
  if (base == 0) {
    base = *s == '0' ? 8 : 10;
  }

  if (base < 2 || base > 36) {
    if (endptr) *endptr = (char*)nptr;  // Invalid base
    return 0;
  }

  cutoff = ULLONG_MAX / (unsigned long long)base;
  cutlim = ULLONG_MAX % (unsigned long long)base;

  for (;; s++) {
    c = *s;
    if (isdigit(c))
      c -= '0';
    else if (isalpha(c))
      c = toupper(c) - 'A' + 10;
    else
      break;

    if (c >= base) break;

    if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim)) {
      any = -1;  // overflow
    } else {
      any = 1;
      acc = acc * base + c;
    }
  }

  if (any < 0) {
    *overflow = 1;
    acc = ULLONG_MAX;
  }

  // Set endptr
  if (endptr) {
    *endptr = (char*)(any ? s : nptr);
  }
  return acc;
}

unsigned long long int strtoull(const char* nptr, char** endptr, int base) {
  int negative;
  int overflow;
  unsigned long long acc =
      strtox_main(nptr, endptr, base, &negative, &overflow);

  if (overflow) return ULLONG_MAX;
  return negative ? -acc : acc;
}

long long int strtoll(const char* nptr, char** endptr, int base) {
  int negative;
  int overflow;
  unsigned long long acc =
      strtox_main(nptr, endptr, base, &negative, &overflow);

  if (overflow) {
    return negative ? LLONG_MIN : LLONG_MAX;
  }

  if (negative) {
    if (acc > (unsigned long long)LLONG_MAX + 1) return LLONG_MIN;
    return -(long long)acc;
  } else {
    if (acc > LLONG_MAX) return LLONG_MAX;
    return (long long)acc;
  }
}

long int strtol(const char* nptr, char** endptr, int base) {
  long long int val = strtoll(nptr, endptr, base);
#if LONG_MAX != LLONG_MAX
  if (val > LONG_MAX) return LONG_MAX;
  if (val < LONG_MIN) return LONG_MIN;
#endif
  return (long int)val;
}

unsigned long int strtoul(const char* nptr, char** endptr, int base) {
  unsigned long long int val = strtoull(nptr, endptr, base);
#if ULONG_MAX != ULLONG_MAX
  if (val > ULONG_MAX) return ULONG_MAX;
#endif
  return (unsigned long int)val;
}

int atoi(const char* nptr) { return (int)strtol(nptr, NULL, 10); }

long int atol(const char* nptr) { return strtol(nptr, NULL, 10); }

long long int atoll(const char* nptr) { return strtoll(nptr, NULL, 10); }

#if (defined(__aarch64__) && defined(__ARM_FP)) || defined(__riscv)
double strtod(const char* nptr, char** endptr) {
  const char* s = nptr;
  double acc = 0.0;
  int sign = 1;

  while (isspace(*s)) s++;

  if (*s == '-') {
    sign = -1;
    s++;
  } else if (*s == '+') {
    s++;
  }

  int any = 0;
  while (isdigit(*s)) {
    any = 1;
    acc = acc * 10.0 + (*s - '0');
    s++;
  }

  if (*s == '.') {
    s++;
    double k = 0.1;
    while (isdigit(*s)) {
      any = 1;
      acc += (*s - '0') * k;
      k *= 0.1;
      s++;
    }
  }

  if (any && (*s == 'e' || *s == 'E')) {
    int esign = 1;
    int exp = 0;
    const char* eptr = s + 1;

    if (*eptr == '-') {
      esign = -1;
      eptr++;
    } else if (*eptr == '+') {
      eptr++;
    }

    if (isdigit(*eptr)) {
      while (isdigit(*eptr)) {
        exp = exp * 10 + (*eptr - '0');
        eptr++;
      }
      s = eptr;
      double p = 1.0;
      double b = 10.0;
      while (exp) {
        if (exp & 1) p *= b;
        b *= b;
        exp >>= 1;
      }
      if (esign > 0)
        acc *= p;
      else
        acc /= p;
    }
  }

  if (endptr) *endptr = (char*)(any ? s : nptr);
  return sign * acc;
}

float strtof(const char* nptr, char** endptr) {
  return (float)strtod(nptr, endptr);
}

double atof(const char* nptr) { return strtod(nptr, NULL); }
#endif

#ifdef __cplusplus
}
#endif
