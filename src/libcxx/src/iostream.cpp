
/**
 * @file iostream
 * @brief C++ 输入输出
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * Based on https://github.com/MRNIU/MiniCRT
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-09-18<td>digmouse233<td>迁移到 doxygen
 * </table>
 */

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

    ostream &ostream::operator<<(const char *lhs) {
        printf("%s", lhs);
        return *this;
    }

    ostream &ostream::operator<<(ostream &(*manip)(ostream &)) {
        return manip(*this);
    }
};
