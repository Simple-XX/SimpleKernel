
// This file is a part of Simple-XX/SimpleKernel
// (https://github.com/Simple-XX/SimpleKernel).
// Based on https://github.com/zouxiaohang/TinySTL
// Functional.hpp for Simple-XX/SimpleKernel.

#ifndef _FUNCTIONAL_HPP_
#define _FUNCTIONAL_HPP_

namespace STL {

    template <class T>
    struct less {
        typedef T    first_argument_type;
        typedef T    second_argument_type;
        typedef bool result_type;

        result_type operator()(const first_argument_type & x,
                               const second_argument_type &y) {
            return x < y;
        }
    };

    template <class T>
    struct equal_to {
        typedef T    first_argument_type;
        typedef T    second_argument_type;
        typedef bool result_type;

        result_type operator()(const first_argument_type & x,
                               const second_argument_type &y) {
            return x == y;
        }
    };

}

#endif /* _FUNCTIONAL_HPP_ */
