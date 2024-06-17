
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

TEST(x8664RflagsInfoNxeTest, ValueTest) {
  EXPECT_EQ(cpu::reginfo::RflagsInfo::If::kBitOffset, 9);
  EXPECT_EQ(cpu::reginfo::RflagsInfo::If::kBitWidth, 1);
  EXPECT_EQ(cpu::reginfo::RflagsInfo::If::kBitMask, 0x200);
  EXPECT_EQ(cpu::reginfo::RflagsInfo::If::kAllSetMask, 1);
}

TEST(x8664Cr0InfoPeTest, ValueTest) {
  EXPECT_EQ(cpu::reginfo::cr::Cr0Info::Pe::kBitOffset, 0);
  EXPECT_EQ(cpu::reginfo::cr::Cr0Info::Pe::kBitWidth, 1);
  EXPECT_EQ(cpu::reginfo::cr::Cr0Info::Pe::kBitMask, 0x1);
  EXPECT_EQ(cpu::reginfo::cr::Cr0Info::Pe::kAllSetMask, 1);
}

TEST(x8664Cr0InfoPgTest, ValueTest) {
  EXPECT_EQ(cpu::reginfo::cr::Cr0Info::Pg::kBitOffset, 31);
  EXPECT_EQ(cpu::reginfo::cr::Cr0Info::Pg::kBitWidth, 1);
  EXPECT_EQ(cpu::reginfo::cr::Cr0Info::Pg::kBitMask, 0x80000000);
  EXPECT_EQ(cpu::reginfo::cr::Cr0Info::Pg::kAllSetMask, 1);
}

TEST(x8664Cr3InfoPwtTest, ValueTest) {
  EXPECT_EQ(cpu::reginfo::cr::Cr3Info::Pwt::kBitOffset, 3);
  EXPECT_EQ(cpu::reginfo::cr::Cr3Info::Pwt::kBitWidth, 1);
  EXPECT_EQ(cpu::reginfo::cr::Cr3Info::Pwt::kBitMask, 0x8);
  EXPECT_EQ(cpu::reginfo::cr::Cr3Info::Pwt::kAllSetMask, 1);
}

TEST(x8664Cr0InfoPcdTest, ValueTest) {
  EXPECT_EQ(cpu::reginfo::cr::Cr3Info::Pcd::kBitOffset, 4);
  EXPECT_EQ(cpu::reginfo::cr::Cr3Info::Pcd::kBitWidth, 1);
  EXPECT_EQ(cpu::reginfo::cr::Cr3Info::Pcd::kBitMask, 0x10);
  EXPECT_EQ(cpu::reginfo::cr::Cr3Info::Pcd::kAllSetMask, 1);
}
TEST(x8664Cr3InfoPageDirectoryBaseTest, ValueTest) {
  EXPECT_EQ(cpu::reginfo::cr::Cr3Info::PageDirectoryBase::kBitOffset, 12);
  EXPECT_EQ(cpu::reginfo::cr::Cr3Info::PageDirectoryBase::kBitWidth, 52);
  EXPECT_EQ(cpu::reginfo::cr::Cr3Info::PageDirectoryBase::kBitMask, ~0xFFF);
  EXPECT_EQ(cpu::reginfo::cr::Cr3Info::PageDirectoryBase::kAllSetMask, 0xFFFFFFFFFFFFF);
}

TEST(x8664Cr4InfoPaeTest, ValueTest) {
  EXPECT_EQ(cpu::reginfo::cr::Cr4Info::Pae::kBitOffset, 5);
  EXPECT_EQ(cpu::reginfo::cr::Cr4Info::Pae::kBitWidth, 1);
  EXPECT_EQ(cpu::reginfo::cr::Cr4Info::Pae::kBitMask, 0x20);
  EXPECT_EQ(cpu::reginfo::cr::Cr4Info::Pae::kAllSetMask, 1);
}
