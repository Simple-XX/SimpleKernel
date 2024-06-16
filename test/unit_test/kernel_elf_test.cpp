
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

#include "kernel_elf.hpp"

#include <gtest/gtest.h>

#include "kernel.elf.h"

TEST(KernelElfTest, DefaultConstructorTest) {
  KernelElf kerlen_elf;
  EXPECT_EQ(kerlen_elf.strtab_, nullptr);
  EXPECT_EQ(kerlen_elf.symtab_.size(), 0);
}

TEST(KernelElfTest, ConstructorWithElfAddrTest) {
  KernelElf kerlen_elf((uint64_t)kernel_elf_data);
  EXPECT_EQ((uint64_t)kerlen_elf.strtab_,
            (uint64_t)((uint64_t)&kernel_elf_data + 0x38d08));
  EXPECT_EQ(kerlen_elf.symtab_.size(), 341);
}

TEST(KernelElfTest, ConstructorWithElfAddrAndElfSizeTest) {
  KernelElf kerlen_elf((uint64_t)kernel_elf_data, kernel_elf_data_len);
  EXPECT_EQ((uint64_t)kerlen_elf.strtab_,
            (uint64_t)((uint64_t)&kernel_elf_data + 0x38d08));
  EXPECT_EQ(kerlen_elf.symtab_.size(), 341);
}

TEST(KernelElfTest, DefaultAssignConstructorTest) {
  KernelElf kerlen_elf;
  KernelElf kerlen_elf2((uint64_t)kernel_elf_data);
  EXPECT_EQ(kerlen_elf.strtab_, nullptr);
  EXPECT_EQ(kerlen_elf.symtab_.size(), 0);

  kerlen_elf = kerlen_elf2;

  EXPECT_EQ((uint64_t)kerlen_elf.strtab_,
            (uint64_t)((uint64_t)&kernel_elf_data + 0x38d08));
  EXPECT_EQ(kerlen_elf.symtab_.size(), 341);
}

TEST(KernelElfTest, DefaultCopyConstructorTest) {
  KernelElf kerlen_elf((uint64_t)kernel_elf_data);

  KernelElf kerlen_elf2(kerlen_elf);

  EXPECT_EQ((uint64_t)kerlen_elf2.strtab_,
            (uint64_t)((uint64_t)&kernel_elf_data + 0x38d08));
  EXPECT_EQ(kerlen_elf2.symtab_.size(), 341);
}
