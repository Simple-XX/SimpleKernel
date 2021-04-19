
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on https://github.com/zouxiaohang/TinySTL
// bitmap.hpp for Simple-XX/SimpleKernel.

#ifndef _BITMAP_HPP_
#define _BITMAP_HPP_

#include "stdlib.h"
#include "stdio.h"

template <size_t N>
class bitmap {
private:
    uint8_t *    start_;
    uint8_t *    finish_;
    const size_t size_;
    const size_t sizeOfUINT8_;
    enum EAlign { ALIGN = 8 };
    size_t  roundUp8(size_t bytes);
    void    setNthInInt8(uint8_t &i, size_t nth, bool newVal);
    uint8_t getMask(uint8_t i, size_t nth) const;
    size_t  getNth(size_t n) const;
    size_t  getMth(size_t n) const;
    void    allocateAndFillN(size_t n, uint8_t val);
    void    THROW(size_t n) const;

public:
    bitmap(void);

    // Returns the number of bits in the bitset that are set (i.e., that
    // have a value of one)
    size_t count(void) const;
    size_t size(void) const;
    // Returns whether the bit at position pos is set (i.e., whether it is
    // one).
    bool test(size_t pos) const;
    // Returns whether any of the bits is set (i.e., whether at least one
    // bit in the bitset is set to one).
    bool any(void) const;
    // Returns whether none of the bits is set (i.e., whether all bits in
    // the bitset have a value of zero).
    bool none(void) const;
    // Returns whether all of the bits in the bitset are set (to one).
    bool all(void) const;

    bitmap &set(void);
    bitmap &set(size_t pos, bool val = true);
    bitmap &reset(void);
    bitmap &reset(size_t pos);
    bitmap &flip(void);
    bitmap &flip(size_t pos);

    size_t to_string(char *buf, size_t size) const;
};

template <size_t N>
bitmap<N>::bitmap(void) : size_(roundUp8(N)), sizeOfUINT8_(roundUp8(N) / 8) {
    allocateAndFillN(sizeOfUINT8_, 0);
}

template <size_t N>
size_t bitmap<N>::roundUp8(size_t bytes) {
    return ((bytes + EAlign::ALIGN - 1) & ~(EAlign::ALIGN - 1));
}

template <size_t N>
void bitmap<N>::setNthInInt8(uint8_t &i, size_t nth, bool newVal) {
    uint8_t temp = getMask(i, nth);
    if ((bool)temp == newVal) {
        return;
    }
    else {
        if (temp) {
            temp = ~temp;
            i    = i & temp;
        }
        else {
            i = i | (1 << nth);
        }
    }
}

template <size_t N>
uint8_t bitmap<N>::getMask(uint8_t i, size_t nth) const {
    return (i & (1 << nth));
}

template <size_t N>
size_t bitmap<N>::getNth(size_t n) const {
    return (n / 8);
}

template <size_t N>
size_t bitmap<N>::getMth(size_t n) const {
    return (n % EAlign::ALIGN);
}

template <size_t N>
void bitmap<N>::allocateAndFillN(size_t n, uint8_t val) {
    start_ = (uint8_t *)malloc(n);
    memset(start_, val, n);
    finish_ = start_ + n;
}

template <size_t N>
void bitmap<N>::THROW(size_t n) const {
    if (n >= size()) {
        err("Out Of Range\n");
    }
}

template <size_t N>
size_t bitmap<N>::count(void) const {
    uint8_t *ptr = start_;
    size_t   sum = 0;
    for (; ptr != finish_; ++ptr) {
        for (int i = 0; i != 8; ++i) {
            uint8_t t = getMask(*ptr, i);
            if (t) {
                ++sum;
            }
        }
    }
    return sum;
}

template <size_t N>
size_t bitmap<N>::size(void) const {
    return size_;
}

template <size_t N>
bool bitmap<N>::test(size_t pos) const {
    THROW(pos);
    const auto nth  = getNth(pos);
    const auto mth  = getMth(pos);
    uint8_t *  ptr  = start_ + nth;
    uint8_t    temp = getMask(*ptr, mth);
    if (temp)
        return true;
    return false;
}

template <size_t N>
bool bitmap<N>::any(void) const {
    uint8_t *ptr = start_;
    for (; ptr != finish_; ++ptr) {
        uint8_t n = *ptr;
        if (n != 0)
            return true;
    }
    return false;
}

template <size_t N>
bool bitmap<N>::none(void) const {
    return !any();
}

template <size_t N>
bool bitmap<N>::all(void) const {
    uint8_t *ptr = start_;
    for (; ptr != finish_; ++ptr) {
        uint8_t n = *ptr;
        if (n != (uint8_t)~0)
            return false;
    }
    return true;
}

template <size_t N>
bitmap<N> &bitmap<N>::set(void) {
    memset(start_, ~0, sizeOfUINT8_);
    return *this;
}

template <size_t N>
bitmap<N> &bitmap<N>::set(size_t pos, bool val) {
    THROW(pos);
    const auto nth = getNth(pos);
    const auto mth = getMth(pos);
    uint8_t *  ptr = start_ + nth; // get the nth uint8_t
    setNthInInt8(*ptr, mth, val);
    return *this;
}

template <size_t N>
bitmap<N> &bitmap<N>::reset(void) {
    memset(start_, 0, sizeOfUINT8_);
    return *this;
}

template <size_t N>
bitmap<N> &bitmap<N>::reset(size_t pos) {
    set(pos, false);
    return *this;
}

template <size_t N>
bitmap<N> &bitmap<N>::flip(void) {
    uint8_t *ptr = start_;
    for (; ptr != finish_; ++ptr) {
        uint8_t n = *ptr;
        *ptr      = ~n;
    }
    return *this;
}

template <size_t N>
bitmap<N> &bitmap<N>::flip(size_t pos) {
    THROW(pos);
    const auto nth  = getNth(pos);
    const auto mth  = getMth(pos);
    uint8_t *  ptr  = start_ + nth;
    uint8_t    temp = getMask(*ptr, mth);
    if (temp) {
        setNthInInt8(*ptr, mth, false);
    }
    else {
        setNthInInt8(*ptr, mth, true);
    }
    return *this;
}

template <size_t N>
size_t bitmap<N>::to_string(char *buf, size_t size) const {
    uint8_t *ptr   = start_;
    size_t   count = 0;
    for (; ptr != finish_; ++ptr) {
        uint8_t n = *ptr;
        for (int i = 0; i != 8; ++i) {
            uint8_t t = getMask(n, i);
            if (t) {
                buf[count++] = '1';
            }
            else {
                buf[count++] = '0';
            }
            if (count > size) {
                err("buf size too small\n");
            }
        }
    }
    return count - 1;
}

#endif
