
/**
 * @file new.cpp
 * @brief 基础内存分配
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2022-01-23
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-01-23<td>MRNIU<td>迁移到 doxygen
 * </table>
 */

#include "new"
#include "stdlib.h"

void *operator new(size_t _size) {
    return malloc(_size);
}

void *operator new[](size_t _size) {
    return malloc(_size);
}

void operator delete(void *_p) {
    free(_p);
}

void operator delete(void *_p, size_t) {
    free(_p);
}

void operator delete[](void *_p) {
    free(_p);
}

void operator delete[](void *_p, size_t) {
    free(_p);
}

void *operator new(size_t, void *_p) throw() {
    return _p;
}

void *operator new[](size_t, void *_p) throw() {
    return _p;
}

void operator delete(void *, void *) throw() {
    return;
}

void operator delete[](void *, void *) throw() {
    return;
}

// TODO
void *operator new(size_t _size, std::align_val_t) {
    return malloc(_size);
}

void operator delete(void *_p, std::align_val_t) {
    free(_p);
    return;
}

void *operator new[](size_t _size, std::align_val_t) {
    return malloc(_size);
}

void operator delete[](void *_p, std::align_val_t) {
    free(_p);
    return;
}

void operator delete(void *_p, size_t, std::align_val_t) {
    free(_p);
    return;
}
void operator delete[](void *_p, size_t, std::align_val_t) {
    free(_p);
    return;
}
