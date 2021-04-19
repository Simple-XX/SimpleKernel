
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on https://github.com/zouxiaohang/TinySTL
// UninitializedFunctions.hpp for Simple-XX/SimpleKernel.

#ifndef _UNINITIALIZED_FUNCTIONS_HPP_
#define _UNINITIALIZED_FUNCTIONS_HPP_

#include "Algorithm.hpp"
#include "Construct.hpp"
#include "Iterator.hpp"
#include "TypeTraits.h"

/***************************************************************************/
template <class InputIterator, class ForwardIterator>
ForwardIterator _uninitialized_copy_aux(InputIterator first, InputIterator last,
                                        ForwardIterator result, _true_type);

template <class InputIterator, class ForwardIterator>
ForwardIterator _uninitialized_copy_aux(InputIterator first, InputIterator last,
                                        ForwardIterator result, _false_type);

template <class InputIterator, class ForwardIterator>
ForwardIterator uninitialized_copy(InputIterator first, InputIterator last,
                                   ForwardIterator result) {
    typedef typename _type_traits<
        typename iterator_traits<InputIterator>::value_type>::is_POD_type
        isPODType;
    return _uninitialized_copy_aux(first, last, result, isPODType());
}

template <class InputIterator, class ForwardIterator>
ForwardIterator _uninitialized_copy_aux(InputIterator first, InputIterator last,
                                        ForwardIterator result, _true_type) {
    memcpy(result, first, (last - first) * sizeof(*first));
    return result + (last - first);
}

template <class InputIterator, class ForwardIterator>
ForwardIterator _uninitialized_copy_aux(InputIterator first, InputIterator last,
                                        ForwardIterator result, _false_type) {
    int i = 0;
    for (; first != last; ++first, ++i) {
        construct((result + i), *first);
    }
    return (result + i);
}

/***************************************************************************/
template <class ForwardIterator, class T>
void _uninitialized_fill_aux(ForwardIterator first, ForwardIterator last,
                             const T &value, _true_type);

template <class ForwardIterator, class T>
void _uninitialized_fill_aux(ForwardIterator first, ForwardIterator last,
                             const T &value, _false_type);

template <class ForwardIterator, class T>
void uninitialized_fill(ForwardIterator first, ForwardIterator last,
                        const T &value) {
    typedef typename _type_traits<T>::is_POD_type isPODType;
    _uninitialized_fill_aux(first, last, value, isPODType());
}

template <class ForwardIterator, class T>
void _uninitialized_fill_aux(ForwardIterator first, ForwardIterator last,
                             const T &value, _true_type) {
    fill(first, last, value);
}

template <class ForwardIterator, class T>
void _uninitialized_fill_aux(ForwardIterator first, ForwardIterator last,
                             const T &value, _false_type) {
    for (; first != last; ++first) {
        construct(first, value);
    }
}

/***************************************************************************/
template <class ForwardIterator, class Size, class T>
ForwardIterator _uninitialized_fill_n_aux(ForwardIterator first, Size n,
                                          const T &x, _true_type);

template <class ForwardIterator, class Size, class T>
ForwardIterator _uninitialized_fill_n_aux(ForwardIterator first, Size n,
                                          const T &x, _false_type);

template <class ForwardIterator, class Size, class T>
inline ForwardIterator uninitialized_fill_n(ForwardIterator first, Size n,
                                            const T &x) {
    typedef typename _type_traits<T>::is_POD_type isPODType;
    return _uninitialized_fill_n_aux(first, n, x, isPODType());
}

template <class ForwardIterator, class Size, class T>
ForwardIterator _uninitialized_fill_n_aux(ForwardIterator first, Size n,
                                          const T &x, _true_type) {
    return fill_n(first, n, x);
}

template <class ForwardIterator, class Size, class T>
ForwardIterator _uninitialized_fill_n_aux(ForwardIterator first, Size n,
                                          const T &x, _false_type) {
    int i = 0;
    for (; i != n; ++i) {
        construct((T *)(first + i), x);
    }
    return (first + i);
}

#endif /* _UNINITIALIZED_FUNCTIONS_HPP_ */
