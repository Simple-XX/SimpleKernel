
/**
 * @file new.cpp
 * @brief 内存分配实现
 * @author Zone.N (Zone.Niuzh@hotmail.com)
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

#include "new"
#include "stdlib.h"

// TODO
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

void operator delete[](void *, size_t) {
}

void *operator new(size_t, void *) throw() {
    return nullptr;
}

void *operator new[](size_t, void *) throw() {
    return nullptr;
}

void operator delete(void *, void *) throw() {
    return;
}

void operator delete[](void *, void *) throw() {
    return;
}

void *operator new(size_t, std::align_val_t) {
    return (void *)1;
}

void operator delete(void *, std::align_val_t) {
    return;
}

void *operator new[](size_t, std::align_val_t) {
    return (void *)1;
}

void operator delete[](void *, std::align_val_t) {
    return;
}

void operator delete(void *, size_t, std::align_val_t) {
    return;
}

void operator delete[](void *, size_t, std::align_val_t) {
    return;
}
