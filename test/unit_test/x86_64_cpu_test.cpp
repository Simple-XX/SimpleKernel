
/**
 * @file x86_64_cpu_test.cpp
 * @brief x86_64 cpu 相关测试
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

#include "x86_64/include/cpu.hpp"

TEST(x8664RegInfoBaseTest, ValueTest) {
  EXPECT_EQ(cpu::reginfo::RegInfoBase::kBitOffset, 0);
  EXPECT_EQ(cpu::reginfo::RegInfoBase::kBitWidth, 64);
  EXPECT_EQ(cpu::reginfo::RegInfoBase::kBitMask, 0xFFFFFFFFFFFFFFFF);
  EXPECT_EQ(cpu::reginfo::RegInfoBase::kAllSetMask, 0xFFFFFFFFFFFFFFFF);
}

TEST(x8664RbpInfoTest, ValueTest) {
  EXPECT_EQ(cpu::reginfo::RbpInfo::kBitOffset, 0);
  EXPECT_EQ(cpu::reginfo::RbpInfo::kBitWidth, 64);
  EXPECT_EQ(cpu::reginfo::RbpInfo::kBitMask, 0xFFFFFFFFFFFFFFFF);
  EXPECT_EQ(cpu::reginfo::RbpInfo::kAllSetMask, 0xFFFFFFFFFFFFFFFF);
}

TEST(x8664EferInfoSceTest, ValueTest) {
  EXPECT_EQ(cpu::reginfo::EferInfo::Sce::kBitOffset, 0);
  EXPECT_EQ(cpu::reginfo::EferInfo::Sce::kBitWidth, 1);
  EXPECT_EQ(cpu::reginfo::EferInfo::Sce::kBitMask, 0x1);
  EXPECT_EQ(cpu::reginfo::EferInfo::Sce::kAllSetMask, 1);
}

TEST(x8664EferInfoLmeTest, ValueTest) {
  EXPECT_EQ(cpu::reginfo::EferInfo::Lme::kBitOffset, 8);
  EXPECT_EQ(cpu::reginfo::EferInfo::Lme::kBitWidth, 1);
  EXPECT_EQ(cpu::reginfo::EferInfo::Lme::kBitMask, 0x100);
  EXPECT_EQ(cpu::reginfo::EferInfo::Lme::kAllSetMask, 1);
}

TEST(x8664EferInfoLmaTest, ValueTest) {
  EXPECT_EQ(cpu::reginfo::EferInfo::Lma::kBitOffset, 10);
  EXPECT_EQ(cpu::reginfo::EferInfo::Lma::kBitWidth, 1);
  EXPECT_EQ(cpu::reginfo::EferInfo::Lma::kBitMask, 0x400);
  EXPECT_EQ(cpu::reginfo::EferInfo::Lma::kAllSetMask, 1);
}

TEST(x8664EferInfoNxeTest, ValueTest) {
  EXPECT_EQ(cpu::reginfo::EferInfo::Nxe::kBitOffset, 11);
  EXPECT_EQ(cpu::reginfo::EferInfo::Nxe::kBitWidth, 1);
  EXPECT_EQ(cpu::reginfo::EferInfo::Nxe::kBitMask, 0x800);
  EXPECT_EQ(cpu::reginfo::EferInfo::Nxe::kAllSetMask, 1);
}
