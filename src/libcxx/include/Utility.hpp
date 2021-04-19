
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on https://github.com/zouxiaohang/TinySTL
// Utility.hpp for Simple-XX/SimpleKernel.

#ifndef _UTILITY_HPP_
#define _UTILITY_HPP_

namespace STL {
    template <class T>
    void swap(T &a, T &b) {
        T temp = a;
        a      = b;
        b      = temp;
    }

    template <class T1, class T2>
    struct pair {
    public:
        typedef T1 first_type;
        typedef T2 second_type;

        T1 first;
        T2 second;

        pair(void);
        template <class U, class V>
        pair(const pair<U, V> &pr);
        pair(const first_type &a, const second_type &b);
        pair &operator=(const pair &pr);
        void  swap(pair &pr);

        template <class U1, class U2>
        friend bool operator==(const pair<U1, U2> &lhs,
                               const pair<U1, U2> &rhs);
        template <class U1, class U2>
        friend bool operator!=(const pair<U1, U2> &lhs,
                               const pair<U1, U2> &rhs);
        template <class U1, class U2>
        friend bool operator<(const pair<U1, U2> &lhs, const pair<U1, U2> &rhs);
        template <class U1, class U2>
        friend bool operator<=(const pair<U1, U2> &lhs,
                               const pair<U1, U2> &rhs);
        template <class U1, class U2>
        friend bool operator>(const pair<U1, U2> &lhs, const pair<U1, U2> &rhs);
        template <class U1, class U2>
        friend bool operator>=(const pair<U1, U2> &lhs,
                               const pair<U1, U2> &rhs);
        template <class U1, class U2>
        friend void swap(pair<U1, U2> &x, pair<U1, U2> &y);
    };

    template <class T1, class T2>
    pair<T1, T2>::pair(void) {
        return;
    }

    template <class T1, class T2>
    template <class U, class V>
    pair<T1, T2>::pair(const pair<U, V> &pr)
        : first(pr.first), second(pr.second) {
    }

    template <class T1, class T2>
    pair<T1, T2>::pair(const first_type &a, const second_type &b)
        : first(a), second(b) {
    }

    template <class T1, class T2>
    pair<T1, T2> &pair<T1, T2>::operator=(const pair<T1, T2> &pr) {
        if (this != &pr) {
            first  = pr.first;
            second = pr.second;
        }
        return *this;
    }

    template <class T1, class T2>
    void pair<T1, T2>::swap(pair<T1, T2> &pr) {
        swap(first, pr.first);
        swap(second, pr.second);
    }

    template <class U1, class U2>
    bool operator==(const pair<U1, U2> &lhs, const pair<U1, U2> &rhs) {
        return lhs.first == rhs.first && lhs.second == rhs.second;
    }

    template <class U1, class U2>
    bool operator!=(const pair<U1, U2> &lhs, const pair<U1, U2> &rhs) {
        return !(lhs == rhs);
    }

    template <class U1, class U2>
    bool operator<(const pair<U1, U2> &lhs, const pair<U1, U2> &rhs) {
        return lhs.first < rhs.first ||
               (!(rhs.first < lhs.first) && lhs.second < rhs.second);
    }

    template <class U1, class U2>
    bool operator<=(const pair<U1, U2> &lhs, const pair<U1, U2> &rhs) {
        return !(rhs < lhs);
    }

    template <class U1, class U2>
    bool operator>(const pair<U1, U2> &lhs, const pair<U1, U2> &rhs) {
        return rhs < lhs;
    }

    template <class U1, class U2>
    bool operator>=(const pair<U1, U2> &lhs, const pair<U1, U2> &rhs) {
        return !(lhs < rhs);
    }

    template <class U1, class U2>
    void swap(pair<U1, U2> &x, pair<U1, U2> &y) {
        x.swap(y);
    }

    // ******* [make_pair] ************
    template <class U, class V>
    pair<U, V> make_pair(const U &u, const V &v) {
        return pair<U, V>(u, v);
    }
}

#endif /* _UTILITY_HPP_ */
