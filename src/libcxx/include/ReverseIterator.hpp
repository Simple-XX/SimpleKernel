
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on https://github.com/zouxiaohang/TinySTL
// ReverseIterator.hpp for Simple-XX/SimpleKernel.

#ifndef _REVERSE_ITERATOR_HPP_
#define _REVERSE_ITERATOR_HPP_

#include "Iterator.hpp"

template <class Iterator>
class reverse_iterator_t {
public:
    typedef Iterator iterator_type;
    typedef
        typename iterator_traits<Iterator>::iterator_category iterator_category;
    typedef typename iterator_traits<Iterator>::value_type    value_type;
    typedef typename iterator_traits<Iterator>::difference_type difference_type;
    typedef typename iterator_traits<Iterator>::pointer         pointer;
    typedef const pointer                                       const_pointer;
    typedef typename iterator_traits<Iterator>::reference       reference;
    typedef const reference                                     const_reference;

private:
    Iterator base_;
    Iterator cur_;
    Iterator advanceNStep(Iterator it, difference_type n, bool right,
                          random_access_iterator_tag);
    Iterator advanceNStep(Iterator it, difference_type n, bool right,
                          bidirectional_iterator_tag);

public:
    reverse_iterator_t(void);
    explicit reverse_iterator_t(const iterator_type &it);
    template <class Iter>
    reverse_iterator_t(const reverse_iterator_t<Iter> &rev_it);
    iterator_type       base(void);
    reference           operator*(void);
    const_reference     operator*(void) const;
    pointer             operator->(void);
    const_pointer       operator->(void) const;
    reverse_iterator_t &operator++(void);
    reverse_iterator_t &operator++(int);
    reverse_iterator_t &operator--(void);
    reverse_iterator_t  operator--(int);
    reference           operator[](difference_type n);
    reverse_iterator_t  operator+(difference_type n) const;
    reverse_iterator_t &operator+=(difference_type n);
    reverse_iterator_t  operator-(difference_type n) const;
    reverse_iterator_t &operator-=(difference_type n);

    template <class T>
    friend bool operator==(const reverse_iterator_t<T> &lhs,
                           const reverse_iterator_t<T> &rhs);
    template <class T>
    friend bool operator!=(const reverse_iterator_t<T> &lhs,
                           const reverse_iterator_t<T> &rhs);
    template <class T>
    friend bool operator<(const reverse_iterator_t<T> &lhs,
                          const reverse_iterator_t<T> &rhs);
    template <class T>
    friend bool operator<=(const reverse_iterator_t<T> &lhs,
                           const reverse_iterator_t<T> &rhs);
    template <class T>
    friend bool operator>(const reverse_iterator_t<T> &lhs,
                          const reverse_iterator_t<T> &rhs);
    template <class T>
    friend bool operator>=(const reverse_iterator_t<T> &lhs,
                           const reverse_iterator_t<T> &rhs);

    template <class T>
    friend reverse_iterator_t<T>
    operator+(typename reverse_iterator_t<T>::difference_type n,
              const reverse_iterator_t<T> &                   rev_it);
    template <class T>
    friend typename reverse_iterator_t<T>::difference_type
    operator-(const reverse_iterator_t<T> &lhs,
              const reverse_iterator_t<T> &rhs);
};

template <class Iterator>
Iterator reverse_iterator_t<Iterator>::advanceNStep(
    Iterator it, difference_type n, bool right, random_access_iterator_tag) {
    if (right) {
        it += n;
    }
    else {
        it -= n;
    }
    return it;
}

template <class Iterator>
Iterator reverse_iterator_t<Iterator>::advanceNStep(
    Iterator it, difference_type n, bool right, bidirectional_iterator_tag) {
    difference_type i;
    difference_type absN = n >= 0 ? n : -n;
    if ((right && n > 0) || (!right && n < 0)) {
        for (i = 0; i != absN; ++i) {
            it = it + 1;
        }
    }
    else if ((!right && n > 0) || (right && n < 0)) {
        for (i = 0; i != absN; ++i) {
            it = it - 1;
        }
    }
    return it;
}

template <class Iterator>
reverse_iterator_t<Iterator>::reverse_iterator_t(void) : base_(0), cur_(0) {
    return;
}

template <class Iterator>
reverse_iterator_t<Iterator>::reverse_iterator_t(const iterator_type &it)
    : base_(it) {
    auto temp = it;
    cur_      = --temp;
    return;
}

template <class Iterator>
template <class Iter>
reverse_iterator_t<Iterator>::reverse_iterator_t(
    const reverse_iterator_t<Iter> &rev_it) {
    base_     = (iterator_type)rev_it.base();
    auto temp = base_;
    cur_      = --temp;
    return;
};

template <class Iterator>
typename reverse_iterator_t<Iterator>::iterator_type
reverse_iterator_t<Iterator>::base(void) {
    return base_;
}

template <class Iterator>
typename reverse_iterator_t<Iterator>::reference
reverse_iterator_t<Iterator>::operator*(void) {
    return (*cur_);
}

template <class Iterator>
typename reverse_iterator_t<Iterator>::const_reference
reverse_iterator_t<Iterator>::operator*(void) const {
    return (*cur_);
}

template <class Iterator>
typename reverse_iterator_t<Iterator>::pointer
reverse_iterator_t<Iterator>::operator->(void) {
    return &(operator*());
}

template <class Iterator>
typename reverse_iterator_t<Iterator>::const_pointer
reverse_iterator_t<Iterator>::operator->(void) const {
    return &(operator*());
}

template <class Iterator>
typename reverse_iterator_t<Iterator>::reverse_iterator_t &
reverse_iterator_t<Iterator>::operator++(void) {
    --base_;
    --cur_;
    return *this;
}

template <class Iterator>
reverse_iterator_t<Iterator> &reverse_iterator_t<Iterator>::operator++(int) {
    reverse_iterator_t temp = *this;
    ++(*this);
    return temp;
}

template <class Iterator>
reverse_iterator_t<Iterator> &reverse_iterator_t<Iterator>::operator--(void) {
    ++base_;
    ++cur_;
    return *this;
}

template <class Iterator>
reverse_iterator_t<Iterator> reverse_iterator_t<Iterator>::operator--(int) {
    reverse_iterator_t temp = *this;
    --(*this);
    return temp;
}

template <class Iterator>
typename reverse_iterator_t<Iterator>::reference
reverse_iterator_t<Iterator>::operator[](difference_type n) {
    return base()[-n - 1];
}

template <class Iterator>
reverse_iterator_t<Iterator>
reverse_iterator_t<Iterator>::operator+(difference_type n) const {
    reverse_iterator_t<Iterator> res = *this;
    res += n;
    return res;
}

template <class Iterator>
reverse_iterator_t<Iterator> &
reverse_iterator_t<Iterator>::operator+=(difference_type n) {
    base_ = advanceNStep(base_, n, false, iterator_category());
    cur_  = advanceNStep(cur_, n, false, iterator_category());
    return *this;
}

template <class Iterator>
reverse_iterator_t<Iterator>
reverse_iterator_t<Iterator>::operator-(difference_type n) const {
    reverse_iterator_t<Iterator> res = *this;
    res -= n;
    return res;
}

template <class Iterator>
reverse_iterator_t<Iterator> &
reverse_iterator_t<Iterator>::operator-=(difference_type n) {
    base_ = advanceNStep(base_, n, true, iterator_category());
    cur_  = advanceNStep(cur_, n, true, iterator_category());
    return *this;
}

template <class T>
bool operator==(const reverse_iterator_t<T> &lhs,
                const reverse_iterator_t<T> &rhs) {
    return lhs.cur_ == rhs.cur_;
}

template <class T>
bool operator!=(const reverse_iterator_t<T> &lhs,
                const reverse_iterator_t<T> &rhs) {
    return !(lhs == rhs);
}

template <class T>
bool operator<(const reverse_iterator_t<T> &lhs,
               const reverse_iterator_t<T> &rhs) {
    return lhs.cur_ < rhs.cur_;
}

template <class T>
bool operator>(const reverse_iterator_t<T> &lhs,
               const reverse_iterator_t<T> &rhs) {
    return lhs.cur_ > rhs.cur_;
}

template <class T>
bool operator>=(const reverse_iterator_t<T> &lhs,
                const reverse_iterator_t<T> &rhs) {
    return !(lhs < rhs);
}

template <class T>
bool operator<=(const reverse_iterator_t<T> &lhs,
                const reverse_iterator_t<T> &rhs) {
    return !(lhs > rhs);
}

template <class T>
reverse_iterator_t<T>
operator+(typename reverse_iterator_t<T>::difference_type n,
          const reverse_iterator_t<T> &                   rev_it) {
    return rev_it + n;
}

template <class T>
typename reverse_iterator_t<T>::difference_type
operator-(const reverse_iterator_t<T> &lhs, const reverse_iterator_t<T> &rhs) {
    return lhs.cur_ - rhs.cur_;
}

#endif /* _REVERSE_ITERATOR_HPP_ */
