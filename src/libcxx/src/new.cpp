
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// new.cpp for Simple-XX/SimpleKernel.

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
