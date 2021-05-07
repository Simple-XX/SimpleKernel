
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
//
// new.cpp for Simple-XX/SimpleKernel.

#include "new"
#include "stdlib.h"

void *operator new(size_t size) {
    return malloc(size);
}

void *operator new[](size_t size) {
    return malloc(size);
}

void operator delete(void *p) {
    free(p);
}

void operator delete(void *p, size_t) {
    free(p);
}

void operator delete[](void *p) {
    free(p);
}

void operator delete[](void *p, size_t) {
    free(p);
}

void *operator new(size_t, void *p) throw() {
    return p;
}

void *operator new[](size_t, void *p) throw() {
    return p;
}

void operator delete(void *, void *) throw() {
    return;
}

void operator delete[](void *, void *) throw() {
    return;
}

// TODO
void *operator new(size_t size, std::align_val_t) {
    return malloc(size);
}

void operator delete(void *p, std::align_val_t) {
    free(p);
    return;
}

void *operator new[](size_t size, std::align_val_t) {
    return malloc(size);
}

void operator delete[](void *p, std::align_val_t) {
    free(p);
    return;
}

void operator delete(void *p, size_t, std::align_val_t) {
    free(p);
    return;
}
void operator delete[](void *p, size_t, std::align_val_t) {
    free(p);
    return;
}
