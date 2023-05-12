
/**
 * @file type_traits.h
 * @brief stl type_traits 支持
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-04-05
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * Based on https://github.com/Alinshans/MyTinySTL
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-04-05<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#ifndef SIMPLEKERNEL_TYPE_TRAITS_H
#define SIMPLEKERNEL_TYPE_TRAITS_H

// 这个头文件用于提取类型信息

// use standard header for type_traits
#include "type_traits"

namespace mystl {

// helper struct

template <class T, T v>
struct m_integral_constant {
    static constexpr T value = v;
};

template <bool b>
using m_bool_constant = m_integral_constant<bool, b>;

typedef m_bool_constant<true>  m_true_type;
typedef m_bool_constant<false> m_false_type;

/*****************************************************************************************/
// type traits

// is_pair

// --- forward declaration begin
template <class T1, class T2>
struct pair;

// --- forward declaration end

template <class T>
struct is_pair : mystl::m_false_type { };

template <class T1, class T2>
struct is_pair<mystl::pair<T1, T2>> : mystl::m_true_type { };

};     // namespace mystl

#endif /* SIMPLEKERNEL_TYPE_TRAITS_H */
