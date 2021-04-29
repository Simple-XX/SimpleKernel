
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on https://github.com/zouxiaohang/TinySTL
// Allocator.hpp for Simple-XX/SimpleKernel.

#ifndef _ALLOCATOR_HPP_
#define _ALLOCATOR_HPP_

#include "Alloc.h"
#include "Construct.hpp"

template <class T>
class allocator {
public:
    typedef T         value_type;
    typedef T *       pointer;
    typedef const T * const_pointer;
    typedef T &       reference;
    typedef const T & const_reference;
    typedef size_t    size_type;
    typedef ptrdiff_t difference_type;

    static T *  allocate();
    static T *  allocate(size_t n);
    static void deallocate(T *ptr);
    static void deallocate(T *ptr, size_t n);

    static void construct(T *ptr);
    static void construct(T *ptr, const T &value);
    static void destroy(T *ptr);
    static void destroy(T *first, T *last);
};

template <class T>
T *allocator<T>::allocate() {
    return static_cast<T *>(alloc::allocate(sizeof(T)));
}

template <class T>
T *allocator<T>::allocate(size_t n) {
    if (n == 0)
        return 0;
    return static_cast<T *>(alloc::allocate(sizeof(T) * n));
}

template <class T>
void allocator<T>::deallocate(T *ptr) {
    alloc::deallocate(static_cast<void *>(ptr), sizeof(T));
}

template <class T>
void allocator<T>::deallocate(T *ptr, size_t n) {
    if (n == 0)
        return;
    alloc::deallocate(static_cast<void *>(ptr), sizeof(T) * n);
}

template <class T>
void allocator<T>::construct(T *ptr) {
    new (ptr) T();
}

template <class T>
void allocator<T>::construct(T *ptr, const T &value) {
    new (ptr) T(value);
}

template <class T>
void allocator<T>::destroy(T *ptr) {
    ptr->~T();
}

template <class T>
void allocator<T>::destroy(T *first, T *last) {
    for (; first != last; ++first) {
        first->~T();
    }
}

#endif /* _ALLOCATOR_HPP_ */
