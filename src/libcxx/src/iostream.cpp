
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on https://github.com/MRNIU/MiniCRT
// iostream.cpp for Simple-XX/SimpleKernel.

#include "stddef.h"
#include "string.h"
#include "stdio.h"
#include "iostream"

namespace std {
    ostream::ostream(void) {
        return;
    }

    ostream::~ostream(void) {
        return;
    }

    ostream &ostream::operator<<(char c) {
        printf("%c", c);
        return *this;
    }

    ostream &ostream::operator<<(int n) {
        printf("%d", n);
        return *this;
    }

    ostream &ostream::operator<<(unsigned int n) {
        printf("%u", n);
        return *this;
    }

    ostream &ostream::operator<<(long n) {
        printf("%d", n);
        return *this;
    }

    ostream &ostream::operator<<(unsigned long n) {
        printf("%u", n);
        return *this;
    }

    ostream &ostream::operator<<(const char *lhs) {
        printf("%s", lhs);
        return *this;
    }

    ostream &ostream::operator<<(ostream &(*manip)(ostream &)) {
        return manip(*this);
    }
};
