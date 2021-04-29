
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on https://github.com/zouxiaohang/TinySTL
// Bitmap.hpp for Simple-XX/SimpleKernel.

#ifndef _BITMAP_HPP_
#define _BITMAP_HPP_

#include "stdint.h"
#include "stddef.h"
#include "iostream"
#include "Allocator.hpp"
#include "string"
#include "UninitializedFunctions.hpp"

namespace STL {
    template <size_t N>
    class bitmap {
    public:
        typedef allocator<uint8_t> dataAllocator;

    private:
        uint8_t *    start_;
        uint8_t *    finish_;
        const size_t size_;
        const size_t sizeOfUINT8_;
        enum EAlign { ALIGN = 8 };

    public:
        bitmap();

        // Returns the number of bits in the bitset that are set (i.e., that
        // have a value of one)
        size_t count() const;
        size_t size() const {
            return size_;
        }
        // Returns whether the bit at position pos is set (i.e., whether it is
        // one).
        bool test(size_t pos) const;
        // Returns whether any of the bits is set (i.e., whether at least one
        // bit in the bitset is set to one).
        bool any() const;
        // Returns whether none of the bits is set (i.e., whether all bits in
        // the bitset have a value of zero).
        bool none() const;
        // Returns whether all of the bits in the bitset are set (to one).
        bool all() const;

        bitmap &set();
        bitmap &set(size_t pos, bool val = true);
        bitmap &reset();
        bitmap &reset(size_t pos);
        bitmap &flip();
        bitmap &flip(size_t pos);

        // std::string to_string() const;
        string to_string() const;

        template <size_t T>
        friend std::ostream &operator<<(std::ostream &os, const bitmap<T> &bm);

    private:
        size_t roundUp8(size_t bytes);

        void setNthInInt8(uint8_t &i, size_t nth, bool newVal);

        uint8_t getMask(uint8_t i, size_t nth) const {
            return (i & (1 << nth));
        }

        size_t getNth(size_t n) const {
            return (n / 8);
        }

        size_t getMth(size_t n) const {
            return (n % EAlign::ALIGN);
        }
        void allocateAndFillN(size_t n, uint8_t val);
        void THROW(size_t n) const;
    };

    template <size_t N>
    void bitmap<N>::allocateAndFillN(size_t n, uint8_t val) {
        start_  = dataAllocator::allocate(n);
        finish_ = uninitialized_fill_n(start_, n, val);
    }
    template <size_t N>
    void bitmap<N>::THROW(size_t n) const {
        if (!(0 <= n && n < size())) {
            err("Out Of Range\n");
        }
    }
    template <size_t N>
    void bitmap<N>::setNthInInt8(uint8_t &i, size_t nth,
                                 bool newVal) { // nth��0��ʼ
        uint8_t temp = getMask(i, nth);
        if ((bool)temp == newVal) {
            return;
        }
        else {
            if (temp) { // nthλΪ1
                temp = ~temp;
                i    = i & temp;
            }
            else { // nthλΪ0
                i = i | (1 << nth);
            }
        }
    }
    template <size_t N>
    size_t bitmap<N>::roundUp8(size_t bytes) {
        return ((bytes + EAlign::ALIGN - 1) & ~(EAlign::ALIGN - 1));
    }
    template <size_t N>
    bitmap<N> &bitmap<N>::set() {
        uninitialized_fill_n(start_, sizeOfUINT8_, ~0);
        return *this;
    }
    template <size_t N>
    bitmap<N> &bitmap<N>::reset() {
        uninitialized_fill_n(start_, sizeOfUINT8_, 0);
        return *this;
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
    bitmap<N>::bitmap() : size_(roundUp8(N)), sizeOfUINT8_(roundUp8(N) / 8) {
        allocateAndFillN(sizeOfUINT8_, 0);
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
    bitmap<N> &bitmap<N>::reset(size_t pos) {
        set(pos, false);
        return *this;
    }
    template <size_t N>
    bitmap<N> &bitmap<N>::flip() {
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
        if (temp)
            setNthInInt8(*ptr, mth, false);
        else
            setNthInInt8(*ptr, mth, true);
        return *this;
    }
    template <size_t N>
    size_t bitmap<N>::count() const {
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
    bool bitmap<N>::any() const {
        uint8_t *ptr = start_;
        for (; ptr != finish_; ++ptr) {
            uint8_t n = *ptr;
            if (n != 0)
                return true;
        }
        return false;
    }
    template <size_t N>
    bool bitmap<N>::all() const {
        uint8_t *ptr = start_;
        for (; ptr != finish_; ++ptr) {
            uint8_t n = *ptr;
            if (n != (uint8_t)~0)
                return false;
        }
        return true;
    }
    template <size_t N>
    bool bitmap<N>::none() const {
        return !any();
    }
    template <size_t N>
    string bitmap<N>::to_string() const {
        string   str;
        uint8_t *ptr = start_;
        for (; ptr != finish_; ++ptr) {
            uint8_t n = *ptr;
            for (int i = 0; i != 8; ++i) {
                uint8_t t = getMask(n, i);
                if (t)
                    str += "1";
                else
                    str += "0";
            }
        }
        return str;
    }

    template <size_t T>
    std::ostream &operator<<(std::ostream &os, const bitmap<T> &bm) {
        os << bm.to_string();
        return os;
    }

}

#endif /* _BITMAP_HPP_ */
