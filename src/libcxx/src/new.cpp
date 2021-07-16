
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// new.cpp for Simple-XX/SimpleKernel.

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
