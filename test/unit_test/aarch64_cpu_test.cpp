
/**
 * @file aarch64_cpu_test.cpp
 * @brief aarch64 cpu 相关测试
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

#include <gtest/gtest.h>

#include "aarch64/include/cpu.hpp"

TEST(Aarch64RegInfoBaseTest, ValueTest) {
  EXPECT_EQ(cpu::reginfo::RegInfoBase::kBitOffset, 0);
  EXPECT_EQ(cpu::reginfo::RegInfoBase::kBitWidth, 64);
  EXPECT_EQ(cpu::reginfo::RegInfoBase::kBitMask, 0xFFFFFFFFFFFFFFFF);
  EXPECT_EQ(cpu::reginfo::RegInfoBase::kAllSetMask, 0xFFFFFFFFFFFFFFFF);
}

TEST(Aarch64X29InfoTest, ValueTest) {
  EXPECT_EQ(cpu::reginfo::X29Info::kBitOffset, 0);
  EXPECT_EQ(cpu::reginfo::X29Info::kBitWidth, 64);
  EXPECT_EQ(cpu::reginfo::X29Info::kBitMask, 0xFFFFFFFFFFFFFFFF);
  EXPECT_EQ(cpu::reginfo::X29Info::kAllSetMask, 0xFFFFFFFFFFFFFFFF);
}
