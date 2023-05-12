
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
#include "cstdlib"

void *operator new(size_t _size) {
    return kmalloc(_size);
}

void *operator new[](size_t _size) {
    return kmalloc(_size);
}

void operator delete(void *_p) {
    kfree(_p);
}

void operator delete(void *_p, size_t) {
    kfree(_p);
}

void operator delete[](void *_p) {
    kfree(_p);
}

void operator delete[](void *_p, size_t) {
    kfree(_p);
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
    return kmalloc(_size);
}

void operator delete(void *_p, std::align_val_t) {
    kfree(_p);
    return;
}

void *operator new[](size_t _size, std::align_val_t) {
    return kmalloc(_size);
}

void operator delete[](void *_p, std::align_val_t) {
    kfree(_p);
    return;
}

void operator delete(void *_p, size_t, std::align_val_t) {
    kfree(_p);
    return;
}
void operator delete[](void *_p, size_t, std::align_val_t) {
    kfree(_p);
    return;
}
