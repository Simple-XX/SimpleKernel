
/**
 * @file kernel_elf_test.cpp
 * @brief elf 解析测试
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-09-02
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-09-02<td>Zone.N<td>创建文件
 * </table>
 */

#include "kernel/include/kernel_elf.hpp"

#include "gtest/gtest.h"

TEST(kernel_elf, DefaultConstructor) {
  auto kerlen_elf = KernelElf();
  EXPECT_EQ(kerlen_elf.strtab_, nullptr);
  EXPECT_EQ(kerlen_elf.symtab_.size(), 0);
}
