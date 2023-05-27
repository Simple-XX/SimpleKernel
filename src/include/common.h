
/**
 * @file common.h
 * @brief 通用定义头文件
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2021-09-18
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2021-09-18<td>digmouse233<td>迁移到 doxygen
 * </table>
 */

#ifndef SIMPLEKERNEL_COMMON_H
#define SIMPLEKERNEL_COMMON_H

#include "cstddef"
#include "cstdint"

namespace COMMON {
// 引用链接脚本中的变量
/// @see http://wiki.osdev.org/Using_Linker_Script_Values
/// 内核开始
extern "C" void*       __executable_start[];
/// 内核结束
extern "C" void*       end[];

/// 内核开始
static const uintptr_t KERNEL_START_ADDR
  = reinterpret_cast<uintptr_t>(__executable_start);
/// 内核结束
static const uintptr_t KERNEL_END_ADDR    = reinterpret_cast<uintptr_t>(end);

// 规定数据大小，方便用
/// 一个字节，8 bits
static constexpr const size_t   BYTE      = 0x1;
/// 1KB
static constexpr const size_t   KB        = 0x400;
/// 1MB
static constexpr const size_t   MB        = 0x100000;
/// 1GB
static constexpr const size_t   GB        = 0x40000000;
/// 页大小 4KB
static constexpr const size_t   PAGE_SIZE = 4 * KB;
/// 内核空间占用大小，包括内核代码部分与预留的，8MB
static constexpr const uint32_t KERNEL_SPACE_SIZE = 8 * MB;
/// 映射内核空间需要的页数
static constexpr const uint64_t KERNEL_SPACE_PAGES
  = KERNEL_SPACE_SIZE / PAGE_SIZE;
/// 栈大小
static constexpr const uintptr_t STACK_SIZE   = 4 * KB;

/// 缓冲区大小，512 字节，一个扇区
static constexpr const uintptr_t BUFFFER_SIZE = 512 * BYTE;

// 页掩码
static constexpr const uintptr_t PAGE_MASK    = ~(PAGE_SIZE - 1);

/**
 * @brief 对齐
 * @tparam T
 * @param  _addr           要对齐的地址
 * @param  _align          要对齐的目标
 * @return T               对齐过的地址
 * @note 向上取整
 * @note 针对指针
 */
template <class T>
inline T ALIGN(const T _addr, const size_t _align) {
    uint8_t* tmp = reinterpret_cast<uint8_t*>(_addr);
    return (T)((ptrdiff_t)(tmp + _align - 1) & (~(_align - 1)));
}

/**
 * @brief 对齐
 * @tparam T
 * @param  _addr           要对齐的整数
 * @param  _align          要对齐的目标
 * @return T               对齐过的整数
 * @note 向上取整
 * @note 针对整数
 */
template <>
inline uint32_t ALIGN(uint32_t _x, size_t _align) {
    return ((_x + _align - 1) & (~(_align - 1)));
}

template <>
inline uint64_t ALIGN(uint64_t _x, size_t _align) {
    return ((_x + _align - 1) & (~(_align - 1)));
}

};     // namespace COMMON

#endif /* SIMPLEKERNEL_COMMON_H */
