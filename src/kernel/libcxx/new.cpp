
/**
 * @file new.cpp
 * @brief 内存分配实现
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-03-31
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-03-31<td>Zone.N<td>迁移到 doxygen
 * </table>
 */

#include "new"

#include <cstddef>

void* operator new(size_t) noexcept { return nullptr; }

void* operator new[](size_t) noexcept { return nullptr; }

void* operator new(size_t, size_t) noexcept { return nullptr; }

void* operator new[](size_t, size_t) noexcept { return nullptr; }

void operator delete(void*) { ; }

void operator delete(void*, size_t) { ; }

void operator delete[](void*) { ; }

void operator delete[](void*, size_t) { ; }
