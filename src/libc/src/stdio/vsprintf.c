
/**
 * @file vsprintf.c
 * @brief vsprintf 定义
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

#ifdef __cplusplus
extern "C" {
#endif

#include "math.h"
#include "stdarg.h"
#include "stdbool.h"
#include "stddef.h"
#include "stdint.h"
#include "stdio.h"

// 'ntoa' conversion buffer size, this must be big enough to hold one converted
// numeric number including padded zeros (dynamically created on stack)
// default: 32 byte
#ifndef PRINTF_NTOA_BUFFER_SIZE
#    define PRINTF_NTOA_BUFFER_SIZE 32U
#endif

// 'ftoa' conversion buffer size, this must be big enough to hold one converted
// float number including padded zeros (dynamically created on stack)
// default: 32 byte
#ifndef PRINTF_FTOA_BUFFER_SIZE
#    define PRINTF_FTOA_BUFFER_SIZE 32U
#endif

// support for the floating point type (%f)
// default: activated
#define PRINTF_DISABLE_SUPPORT_FLOAT
#ifndef PRINTF_DISABLE_SUPPORT_FLOAT
#    define PRINTF_SUPPORT_FLOAT
#endif

// support for exponential floating point notation (%e/%g)
// default: activated
#ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
#    define PRINTF_SUPPORT_EXPONENTIAL
#endif

// define the default floating point precision
// default: 6 digits
#ifndef PRINTF_DEFAULT_FLOAT_PRECISION
#    define PRINTF_DEFAULT_FLOAT_PRECISION 6U
#endif

// define the largest float suitable to print with %f
// default: 1e9
#ifndef PRINTF_MAX_FLOAT
#    define PRINTF_MAX_FLOAT 1e9
#endif

// support for the long long types (%llu or %p)
// default: activated
#ifndef PRINTF_DISABLE_SUPPORT_LONG_LONG
#    define PRINTF_SUPPORT_LONG_LONG
#endif

// support for the ptrdiff_t type (%t)
// ptrdiff_t is normally defined in <stddef.h> as long or long long type
// default: activated
#ifndef PRINTF_DISABLE_SUPPORT_PTRDIFF_T
#    define PRINTF_SUPPORT_PTRDIFF_T
#endif

///////////////////////////////////////////////////////////////////////////////

// internal flag definitions
#define FLAGS_ZEROPAD   (1U << 0U)
#define FLAGS_LEFT      (1U << 1U)
#define FLAGS_PLUS      (1U << 2U)
#define FLAGS_SPACE     (1U << 3U)
#define FLAGS_HASH      (1U << 4U)
#define FLAGS_UPPERCASE (1U << 5U)
#define FLAGS_CHAR      (1U << 6U)
#define FLAGS_SHORT     (1U << 7U)
#define FLAGS_LONG      (1U << 8U)
#define FLAGS_LONG_LONG (1U << 9U)
#define FLAGS_PRECISION (1U << 10U)
#define FLAGS_ADAPT_EXP (1U << 11U)

// import float.h for DBL_MAX
#if defined(PRINTF_SUPPORT_FLOAT)
#    include "float.h"
#endif

// output function type
typedef void (*out_fct_type)(char _character, void* _buffer, size_t _idx,
                             size_t _maxlen);

// wrapper (used as buffer) for output function type
typedef struct {
    void (*fct)(char _character, void* arg);
    void* arg;
} out_fct_wrap_type;

// internal buffer output
inline static void
_out_buffer(char _character, void* _buffer, size_t _idx, size_t _maxlen) {
    if (_idx < _maxlen) {
        ((char*)_buffer)[_idx] = _character;
    }
}

// internal null output
inline static void
_out_null(char _character, void* _buffer, size_t _idx, size_t _maxlen) {
    (void)_character;
    (void)_buffer;
    (void)_idx;
    (void)_maxlen;
}

// internal secure strlen
// \return The length of the string (excluding the terminating 0) limited by
// 'maxsize'
inline static unsigned int _strnlen_s(const char* _str, size_t _maxsize) {
    const char* s;
    for (s = _str; *s && _maxsize--; ++s)
        ;
    return (unsigned int)(s - _str);
}

// internal test if char is a digit (0-9)
// \return true if char is a digit
inline static bool _is_digit(char _ch) {
    return (_ch >= '0') && (_ch <= '9');
}

// internal ASCII string to unsigned int conversion
static unsigned int _atoi(const char** _str) {
    unsigned int i = 0U;
    while (_is_digit(**_str)) {
        i = i * 10U + (unsigned int)(*((*_str)++) - '0');
    }
    return i;
}

// output the specified string in reverse, taking care of any zero-padding
static size_t _out_rev(out_fct_type _out, char* _buffer, size_t _idx,
                       size_t _maxlen, const char* _buf, size_t _len,
                       unsigned int _width, unsigned int _flags) {
    const size_t start_idx = _idx;

    // pad spaces up to given _width
    if (!(_flags & FLAGS_LEFT) && !(_flags & FLAGS_ZEROPAD)) {
        for (size_t i = _len; i < _width; i++) {
            _out(' ', _buffer, _idx++, _maxlen);
        }
    }

    // reverse string
    while (_len) {
        _out(_buf[--_len], _buffer, _idx++, _maxlen);
    }

    // append pad spaces up to given width
    if (_flags & FLAGS_LEFT) {
        while (_idx - start_idx < _width) {
            _out(' ', _buffer, _idx++, _maxlen);
        }
    }

    return _idx;
}

// internal itoa format
static size_t
_ntoa_format(out_fct_type _out, char* _buffer, size_t _idx, size_t _maxlen,
             char* _buf, size_t _len, bool _negative, unsigned int _base,
             unsigned int _prec, unsigned int _width, unsigned int _flags) {
    // pad leading zeros
    if (!(_flags & FLAGS_LEFT)) {
        if (_width && (_flags & FLAGS_ZEROPAD)
            && (_negative || (_flags & (FLAGS_PLUS | FLAGS_SPACE)))) {
            _width--;
        }
        while ((_len < _prec) && (_len < PRINTF_NTOA_BUFFER_SIZE)) {
            _buf[_len++] = '0';
        }
        while ((_flags & FLAGS_ZEROPAD) && (_len < _width)
               && (_len < PRINTF_NTOA_BUFFER_SIZE)) {
            _buf[_len++] = '0';
        }
    }

    // handle hash
    if (_flags & FLAGS_HASH) {
        if (!(_flags & FLAGS_PRECISION) && _len
            && ((_len == _prec) || (_len == _width))) {
            _len--;
            if (_len && (_base == 16U)) {
                _len--;
            }
        }
        if ((_base == 16U) && !(_flags & FLAGS_UPPERCASE)
            && (_len < PRINTF_NTOA_BUFFER_SIZE)) {
            _buf[_len++] = 'x';
        }
        else if ((_base == 16U) && (_flags & FLAGS_UPPERCASE)
                 && (_len < PRINTF_NTOA_BUFFER_SIZE)) {
            _buf[_len++] = 'X';
        }
        else if ((_base == 2U) && (_len < PRINTF_NTOA_BUFFER_SIZE)) {
            _buf[_len++] = 'b';
        }
        if (_len < PRINTF_NTOA_BUFFER_SIZE) {
            _buf[_len++] = '0';
        }
    }

    if (_len < PRINTF_NTOA_BUFFER_SIZE) {
        if (_negative) {
            _buf[_len++] = '-';
        }
        else if (_flags & FLAGS_PLUS) {
            _buf[_len++] = '+';    // ignore the space if the '+' exists
        }
        else if (_flags & FLAGS_SPACE) {
            _buf[_len++] = ' ';
        }
    }

    return _out_rev(_out, _buffer, _idx, _maxlen, _buf, _len, _width, _flags);
}

// internal itoa for 'long' type
static size_t
_ntoa_long(out_fct_type _out, char* _buffer, size_t _idx, size_t _maxlen,
           unsigned long _value, bool _negative, unsigned long _base,
           unsigned int _prec, unsigned int _width, unsigned int _flags) {
    char   buf[PRINTF_NTOA_BUFFER_SIZE];
    size_t len = 0U;

    // no hash for 0 values
    if (!_value) {
        _flags &= ~FLAGS_HASH;
    }

    // write if precision != 0 and _value is != 0
    if (!(_flags & FLAGS_PRECISION) || _value) {
        do {
            const char digit  = (char)(_value % _base);
            buf[len++]        = digit < 10
                                ? '0' + digit
                                : (_flags & FLAGS_UPPERCASE ? 'A' : 'a') + digit - 10;
            _value           /= _base;
        } while (_value && (len < PRINTF_NTOA_BUFFER_SIZE));
    }

    return _ntoa_format(_out, _buffer, _idx, _maxlen, buf, len, _negative,
                        (unsigned int)_base, _prec, _width, _flags);
}

// internal itoa for 'long long' type
#if defined(PRINTF_SUPPORT_LONG_LONG)
static size_t
_ntoa_long_long(out_fct_type _out, char* _buffer, size_t _idx, size_t _maxlen,
                unsigned long long _value, bool _negative,
                unsigned long long _base, unsigned int _prec,
                unsigned int _width, unsigned int _flags) {
    char   buf[PRINTF_NTOA_BUFFER_SIZE];
    size_t len = 0U;

    // no hash for 0 values
    if (!_value) {
        _flags &= ~FLAGS_HASH;
    }

#    if defined(__i386__)
    // write if precision != 0 and _value is != 0
    if (!(_flags & FLAGS_PRECISION) || _value) {
        do {
            unsigned long long no;
            unsigned long long t     = udivmoddi4(_value, _base, &no);
            const char         digit = (char)t;
            buf[len++]               = digit < 10
                                       ? '0' + digit
                                       : (_flags & FLAGS_UPPERCASE ? 'A' : 'a') + digit - 10;
            _value                   = udivdi3(_value, _base);
        } while (_value && (len < PRINTF_NTOA_BUFFER_SIZE));
    }
#    else
    // write if precision != 0 and value is != 0
    if (!(_flags & FLAGS_PRECISION) || _value) {
        do {
            const char digit  = (char)(_value % _base);
            buf[len++]        = digit < 10
                                ? '0' + digit
                                : (_flags & FLAGS_UPPERCASE ? 'A' : 'a') + digit - 10;
            _value           /= _base;
        } while (_value && (len < PRINTF_NTOA_BUFFER_SIZE));
    }
#    endif
    return _ntoa_format(_out, _buffer, _idx, _maxlen, buf, len, _negative,
                        (unsigned int)_base, _prec, _width, _flags);
}
#endif    // PRINTF_SUPPORT_LONG_LONG

#if defined(PRINTF_SUPPORT_FLOAT)

#    if defined(PRINTF_SUPPORT_EXPONENTIAL)
// forward declaration so that _ftoa can switch to exp notation for values >
// PRINTF_MAX_FLOAT
static size_t _etoa(out_fct_type _out, char* _buffer, size_t _idx,
                    size_t _maxlen, double _value, unsigned int _prec,
                    unsigned int _width, unsigned int _flags);
#    endif

// internal ftoa for fixed decimal floating point
static size_t _ftoa(out_fct_type _out, char* _buffer, size_t _idx,
                    size_t _maxlen, double _value, unsigned int _prec,
                    unsigned int _width, unsigned int _flags) {
    char                buf[PRINTF_FTOA_BUFFER_SIZE];
    size_t              len  = 0U;
    double              diff = 0.0;

    // powers of 10
    static const double pow10[]
      = { 1,      10,      100,      1000,      10000,
          100000, 1000000, 10000000, 100000000, 1000000000 };

    // test for special values
    if (_value != _value) {
        return _out_rev(_out, _buffer, _idx, _maxlen, "nan", 3, _width, _flags);
    }
    if (_value < -DBL_MAX) {
        return _out_rev(_out, _buffer, _idx, _maxlen, "fni-", 4, _width,
                        _flags);
    }
    if (_value > DBL_MAX) {
        return _out_rev(_out, _buffer, _idx, _maxlen,
                        (_flags & FLAGS_PLUS) ? "fni+" : "fni",
                        (_flags & FLAGS_PLUS) ? 4U : 3U, _width, _flags);
    }

    // test for very large values
    // standard printf behavior is to print EVERY whole number digit -- which
    // could be 100s of characters overflowing your buffers == bad
    if ((_value > PRINTF_MAX_FLOAT) || (_value < -PRINTF_MAX_FLOAT)) {
#    if defined(PRINTF_SUPPORT_EXPONENTIAL)
        return _etoa(_out, _buffer, _idx, _maxlen, _value, _prec, _width,
                     _flags);
#    else
        return 0U;
#    endif
    }

    // test for negative
    bool negative = false;
    if (_value < 0) {
        negative = true;
        _value   = 0 - _value;
    }

    // set default precision, if not set explicitly
    if (!(_flags & FLAGS_PRECISION)) {
        _prec = PRINTF_DEFAULT_FLOAT_PRECISION;
    }
    // limit precision to 9, cause a prec >= 10 can lead to overflow errors
    while ((len < PRINTF_FTOA_BUFFER_SIZE) && (_prec > 9U)) {
        buf[len++] = '0';
        _prec--;
    }

    int           whole = (int)_value;
    double        tmp   = (_value - whole) * pow10[_prec];
    unsigned long frac  = (unsigned long)tmp;
    diff                = tmp - frac;

    if (diff > 0.5) {
        ++frac;
        // handle rollover, e.g. case 0.99 with prec 1 is 1.0
        if (frac >= pow10[prec]) {
            frac = 0;
            ++whole;
        }
    }
    else if (diff < 0.5) {
    }
    else if ((frac == 0U) || (frac & 1U)) {
        // if halfway, round up if odd OR if last digit is 0
        ++frac;
    }

    if (prec == 0U) {
        diff = _value - (double)whole;
        if ((!(diff < 0.5) || (diff > 0.5)) && (whole & 1)) {
            // exactly 0.5 and ODD, then round up
            // 1.5 -> 2, but 2.5 -> 2
            ++whole;
        }
    }
    else {
        unsigned int count = prec;
        // now do fractional part, as an unsigned number
        while (len < PRINTF_FTOA_BUFFER_SIZE) {
            --count;
            buf[len++] = (char)(48U + (frac % 10U));
            if (!(frac /= 10U)) {
                break;
            }
        }
        // add extra 0s
        while ((len < PRINTF_FTOA_BUFFER_SIZE) && (count-- > 0U)) {
            buf[len++] = '0';
        }
        if (len < PRINTF_FTOA_BUFFER_SIZE) {
            // add decimal
            buf[len++] = '.';
        }
    }

    // do whole part, number is reversed
    while (len < PRINTF_FTOA_BUFFER_SIZE) {
        buf[len++] = (char)(48 + (whole % 10));
        if (!(whole /= 10)) {
            break;
        }
    }

    // pad leading zeros
    if (!(_flags & FLAGS_LEFT) && (_flags & FLAGS_ZEROPAD)) {
        if (_width && (negative || (_flags & (FLAGS_PLUS | FLAGS_SPACE)))) {
            _width--;
        }
        while ((len < _width) && (len < PRINTF_FTOA_BUFFER_SIZE)) {
            buf[len++] = '0';
        }
    }

    if (len < PRINTF_FTOA_BUFFER_SIZE) {
        if (negative) {
            buf[len++] = '-';
        }
        else if (_flags & FLAGS_PLUS) {
            buf[len++] = '+';    // ignore the space if the '+' exists
        }
        else if (_flags & FLAGS_SPACE) {
            buf[len++] = ' ';
        }
    }

    return _out_rev(_out, _buffer, _idx, _maxlen, buf, len, _width, _flags);
}

#    if defined(PRINTF_SUPPORT_EXPONENTIAL)
// internal ftoa variant for exponential floating-point type, contributed by
// Martijn Jasperse <m.jasperse@gmail.com>
static size_t _etoa(out_fct_type _out, char* _buffer, size_t _idx,
                    size_t _maxlen, double _value, unsigned int _prec,
                    unsigned int _width, unsigned int _flags) {
    // check for NaN and special values
    if ((_value != _value) || (_value > DBL_MAX) || (_value < -DBL_MAX)) {
        return _ftoa(_out, _buffer, _idx, _maxlen, _value, _prec, _width,
                     _flags);
    }

    // determine the sign
    const bool negative = _value < 0;
    if (negative) {
        _value = -_value;
    }

    // default precision
    if (!(_flags & FLAGS_PRECISION)) {
        _prec = PRINTF_DEFAULT_FLOAT_PRECISION;
    }

    // determine the decimal exponent
    // based on the algorithm by David Gay
    // (https://www.ampl.com/netlib/fp/dtoa.c)
    union {
        uint64_t U;
        double   F;
    } conv;

    conv.F   = _value;
    int exp2 = (int)((conv.U >> 52U) & 0x07FFU) - 1023;    // effectively log2
    conv.U   = (conv.U & ((1ULL << 52U) - 1U))
           | (1023ULL << 52U);    // drop the exponent so conv.F is now in [1,2)
    // now approximate log10 from the log2 integer part and an expansion of ln
    // around 1.5
    int expval       = (int)(0.1760912590558 + exp2 * 0.301029995663981
                       + (conv.F - 1.5) * 0.289529654602168);
    // now we want to compute 10^expval but we want to be sure it won't overflow
    exp2             = (int)(expval * 3.321928094887362 + 0.5);
    const double z   = expval * 2.302585092994046 - exp2 * 0.6931471805599453;
    const double z2  = z * z;
    conv.U           = (uint64_t)(exp2 + 1023) << 52U;
    // compute exp(z) using continued fractions, see
    // https://en.wikipedia.org/wiki/Exponential_function#Continued_fractions_for_ex
    conv.F          *= 1 + 2 * z / (2 - z + (z2 / (6 + (z2 / (10 + z2 / 14)))));
    // correct for rounding errors
    if (_value < conv.F) {
        expval--;
        conv.F /= 10;
    }

    // the exponent format is "%+03d" and largest value is "307", so set aside
    // 4-5 characters
    unsigned int minwidth = ((expval < 100) && (expval > -100)) ? 4U : 5U;

    // in "%g" mode, "prec" is the number of *significant figures* not decimals
    if (_flags & FLAGS_ADAPT_EXP) {
        // do we want to fall-back to "%f" mode?
        if ((_value >= 1e-4) && (_value < 1e6)) {
            if ((int)_prec > expval) {
                _prec = (unsigned)((int)_prec - expval - 1);
            }
            else {
                _prec = 0;
            }
            _flags |= FLAGS_PRECISION;    // make sure _ftoa respects precision
            // no characters in exponent
            minwidth = 0U;
            expval   = 0;
        }
        else {
            // we use one sigfig for the whole part
            if ((_prec > 0) && (_flags & FLAGS_PRECISION)) {
                --_prec;
            }
        }
    }

    // will everything fit?
    unsigned int fwidth = _width;
    if (_width > minwidth) {
        // we didn't fall-back so subtract the characters required for the
        // exponent
        fwidth -= minwidth;
    }
    else {
        // not enough characters, so go back to default sizing
        fwidth = 0U;
    }
    if ((_flags & FLAGS_LEFT) && minwidth) {
        // if we're padding on the right, DON'T pad the floating part
        fwidth = 0U;
    }

    // rescale the float value
    if (expval) {
        _value /= conv.F;
    }

    // output the floating part
    const size_t start_idx = _idx;
    _idx = _ftoa(_out, _buffer, _idx, _maxlen, negative ? -_value : _value,
                 _prec, fwidth, _flags & ~FLAGS_ADAPT_EXP);

    // output the exponent part
    if (minwidth) {
        // output the exponential symbol
        _out((_flags & FLAGS_UPPERCASE) ? 'E' : 'e', _buffer, _idx++, _maxlen);
        // output the exponent value
        _idx = _ntoa_long(_out, _buffer, _idx, _maxlen,
                          (expval < 0) ? -expval : expval, expval < 0, 10, 0,
                          minwidth - 1, FLAGS_ZEROPAD | FLAGS_PLUS);
        // might need to right-pad spaces
        if (_flags & FLAGS_LEFT) {
            while (_idx - start_idx < _width) {
                _out(' ', _buffer, _idx++, _maxlen);
            }
        }
    }
    return _idx;
}

#    endif /* PRINTF_SUPPORT_EXPONENTIAL */
#endif     /* PRINTF_SUPPORT_FLOAT */

// internal vsnprintf
int _vsnprintf(char* _buffer, const size_t _maxlen, const char* _format,
               va_list _va) {
    out_fct_type out = _out_buffer;
    unsigned int flags, width, precision, n;
    size_t       idx = 0U;

    if (!_buffer) {
        // use null output function
        out = _out_null;
    }

    while (*_format) {
        // _format specifier?  %[flags][width][.precision][length]
        if (*_format != '%') {
            // no
            out(*_format, _buffer, idx++, _maxlen);
            _format++;
            continue;
        }
        else {
            // yes, evaluate it
            _format++;
        }

        // evaluate flags
        flags = 0U;
        do {
            switch (*_format) {
                case '0':
                    flags |= FLAGS_ZEROPAD;
                    _format++;
                    n = 1U;
                    break;
                case '-':
                    flags |= FLAGS_LEFT;
                    _format++;
                    n = 1U;
                    break;
                case '+':
                    flags |= FLAGS_PLUS;
                    _format++;
                    n = 1U;
                    break;
                case ' ':
                    flags |= FLAGS_SPACE;
                    _format++;
                    n = 1U;
                    break;
                case '#':
                    flags |= FLAGS_HASH;
                    _format++;
                    n = 1U;
                    break;
                default:
                    n = 0U;
                    break;
            }
        } while (n);

        // evaluate width field
        width = 0U;
        if (_is_digit(*_format)) {
            width = _atoi(&_format);
        }
        else if (*_format == '*') {
            const int w = va_arg(_va, int);
            if (w < 0) {
                flags |= FLAGS_LEFT;    // reverse padding
                width  = (unsigned int)-w;
            }
            else {
                width = (unsigned int)w;
            }
            _format++;
        }

        // evaluate precision field
        precision = 0U;
        if (*_format == '.') {
            flags |= FLAGS_PRECISION;
            _format++;
            if (_is_digit(*_format)) {
                precision = _atoi(&_format);
            }
            else if (*_format == '*') {
                const int prec = (int)va_arg(_va, int);
                precision      = prec > 0 ? (unsigned int)prec : 0U;
                _format++;
            }
        }

        // evaluate length field
        switch (*_format) {
            case 'l':
                flags |= FLAGS_LONG;
                _format++;
                if (*_format == 'l') {
                    flags |= FLAGS_LONG_LONG;
                    _format++;
                }
                break;
            case 'h':
                flags |= FLAGS_SHORT;
                _format++;
                if (*_format == 'h') {
                    flags |= FLAGS_CHAR;
                    _format++;
                }
                break;
#if defined(PRINTF_SUPPORT_PTRDIFF_T)
            case 't':
                flags |= (sizeof(ptrdiff_t) == sizeof(long) ? FLAGS_LONG
                                                            : FLAGS_LONG_LONG);
                _format++;
                break;
#endif
            case 'j':
                flags |= (sizeof(intmax_t) == sizeof(long) ? FLAGS_LONG
                                                           : FLAGS_LONG_LONG);
                _format++;
                break;
            case 'z':
                flags |= (sizeof(size_t) == sizeof(long) ? FLAGS_LONG
                                                         : FLAGS_LONG_LONG);
                _format++;
                break;
            default:
                break;
        }

        // evaluate specifier
        switch (*_format) {
            case 'd':
            case 'i':
            case 'u':
            case 'x':
            case 'X':
            case 'o':
            case 'b': {
                // set the base
                unsigned int base;
                if (*_format == 'x' || *_format == 'X') {
                    base = 16U;
                }
                else if (*_format == 'o') {
                    base = 8U;
                }
                else if (*_format == 'b') {
                    base = 2U;
                }
                else {
                    base   = 10U;
                    flags &= ~FLAGS_HASH;    // no hash for dec _format
                }
                // uppercase
                if (*_format == 'X') {
                    flags |= FLAGS_UPPERCASE;
                }

                // no plus or space flag for u, x, X, o, b
                if ((*_format != 'i') && (*_format != 'd')) {
                    flags &= ~(FLAGS_PLUS | FLAGS_SPACE);
                }

                // ignore '0' flag when precision is given
                if (flags & FLAGS_PRECISION) {
                    flags &= ~FLAGS_ZEROPAD;
                }

                // convert the integer
                if ((*_format == 'i') || (*_format == 'd')) {
                    // signed
                    if (flags & FLAGS_LONG_LONG) {
#if defined(PRINTF_SUPPORT_LONG_LONG)
                        const long long value = va_arg(_va, long long);
                        idx                   = _ntoa_long_long(
                          out, _buffer, idx, _maxlen,
                          (unsigned long long)(value > 0 ? value : 0 - value),
                          value < 0, base, precision, width, flags);
#endif
                    }
                    else if (flags & FLAGS_LONG) {
                        const long value = va_arg(_va, long);
                        idx = _ntoa_long(out, _buffer, idx, _maxlen,
                                         (unsigned long)(value > 0 ? value
                                                                   : 0 - value),
                                         value < 0, base, precision, width,
                                         flags);
                    }
                    else {
                        const int value
                          = (flags & FLAGS_CHAR)  ? (char)va_arg(_va, int)
                          : (flags & FLAGS_SHORT) ? (short int)va_arg(_va, int)
                                                  : va_arg(_va, int);
                        idx = _ntoa_long(out, _buffer, idx, _maxlen,
                                         (unsigned int)(value > 0 ? value
                                                                  : 0 - value),
                                         value < 0, base, precision, width,
                                         flags);
                    }
                }
                else {
                    // unsigned
                    if (flags & FLAGS_LONG_LONG) {
#if defined(PRINTF_SUPPORT_LONG_LONG)
                        idx = _ntoa_long_long(out, _buffer, idx, _maxlen,
                                              va_arg(_va, unsigned long long),
                                              false, base, precision, width,
                                              flags);
#endif
                    }
                    else if (flags & FLAGS_LONG) {
                        idx = _ntoa_long(out, _buffer, idx, _maxlen,
                                         va_arg(_va, unsigned long), false,
                                         base, precision, width, flags);
                    }
                    else {
                        const unsigned int value
                          = (flags & FLAGS_CHAR)
                            ? (unsigned char)va_arg(_va, unsigned int)
                          : (flags & FLAGS_SHORT)
                            ? (unsigned short int)va_arg(_va, unsigned int)
                            : va_arg(_va, unsigned int);
                        idx = _ntoa_long(out, _buffer, idx, _maxlen, value,
                                         false, base, precision, width, flags);
                    }
                }
                _format++;
                break;
            }
#if defined(PRINTF_SUPPORT_FLOAT)
            case 'f':
            case 'F':
                if (*_format == 'F') {
                    flags |= FLAGS_UPPERCASE;
                }
                idx = _ftoa(out, _buffer, idx, _maxlen, va_arg(_va, double),
                            precision, width, flags);
                _format++;
                break;
#    if defined(PRINTF_SUPPORT_EXPONENTIAL)
            case 'e':
            case 'E':
            case 'g':
            case 'G':
                if ((*_format == 'g') || (*_format == 'G')) {
                    flags |= FLAGS_ADAPT_EXP;
                }
                if ((*_format == 'E') || (*_format == 'G')) {
                    flags |= FLAGS_UPPERCASE;
                }
                idx = _etoa(out, _buffer, idx, _maxlen, va_arg(_va, double),
                            precision, width, flags);
                _format++;
                break;
#    endif    // PRINTF_SUPPORT_EXPONENTIAL
#endif        // PRINTF_SUPPORT_FLOAT
            case 'c': {
                unsigned int l = 1U;
                // pre padding
                if (!(flags & FLAGS_LEFT)) {
                    while (l++ < width) {
                        out(' ', _buffer, idx++, _maxlen);
                    }
                }
                // char output
                out((char)va_arg(_va, int), _buffer, idx++, _maxlen);
                // post padding
                if (flags & FLAGS_LEFT) {
                    while (l++ < width) {
                        out(' ', _buffer, idx++, _maxlen);
                    }
                }
                _format++;
                break;
            }

            case 's': {
                const char*  p = va_arg(_va, char*);
                unsigned int l
                  = _strnlen_s(p, precision ? precision : (size_t)-1);
                // pre padding
                if (flags & FLAGS_PRECISION) {
                    l = (l < precision ? l : precision);
                }
                if (!(flags & FLAGS_LEFT)) {
                    while (l++ < width) {
                        out(' ', _buffer, idx++, _maxlen);
                    }
                }
                // string output
                while ((*p != 0)
                       && (!(flags & FLAGS_PRECISION) || precision--)) {
                    out(*(p++), _buffer, idx++, _maxlen);
                }
                // post padding
                if (flags & FLAGS_LEFT) {
                    while (l++ < width) {
                        out(' ', _buffer, idx++, _maxlen);
                    }
                }
                _format++;
                break;
            }

            case 'p': {
                width  = sizeof(void*) * 2U;
                flags |= FLAGS_ZEROPAD | FLAGS_UPPERCASE;
#if defined(PRINTF_SUPPORT_LONG_LONG)
                const bool is_ll = sizeof(uintptr_t) == sizeof(long long);
                if (is_ll) {
                    idx = _ntoa_long_long(out, _buffer, idx, _maxlen,
                                          (unsigned long)va_arg(_va, void*),
                                          false, 16U, precision, width, flags);
                }
                else {
#endif
                    idx = _ntoa_long(
                      out, _buffer, idx, _maxlen,
                      (unsigned long)((unsigned long)va_arg(_va, void*)), false,
                      16U, precision, width, flags);
#if defined(PRINTF_SUPPORT_LONG_LONG)
                }
#endif
                _format++;
                break;
            }

            case '%':
                out('%', _buffer, idx++, _maxlen);
                _format++;
                break;

            default:
                out(*_format, _buffer, idx++, _maxlen);
                _format++;
                break;
        }
    }

    // termination
    out((char)0, _buffer, idx < _maxlen ? idx : _maxlen - 1U, _maxlen);

    // return written chars without terminating \0
    return (int)idx;
}

int sprintf_(char* _buffer, const char* _format, ...) {
    va_list va;
    va_start(va, _format);
    // const int ret = _vsnprintf(_buffer, (size_t)-1, _format, va);
    const int ret = _vsnprintf(_buffer, (size_t)-1, "TEST:%d\n", va);
    va_end(va);
    return ret;
}

int snprintf_(char* _buffer, size_t _count, const char* _format, ...) {
    va_list va;
    va_start(va, _format);
    const int ret = _vsnprintf(_buffer, _count, _format, va);
    va_end(va);
    return ret;
}

int vsnprintf_(char* _buffer, size_t _count, const char* _format, va_list _va) {
    return _vsnprintf(_buffer, _count, _format, _va);
}

#ifdef __cplusplus
}
#endif
