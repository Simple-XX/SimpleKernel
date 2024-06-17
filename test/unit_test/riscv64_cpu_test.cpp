
/**
 * @file riscv64_cpu_test.cpp
 * @brief riscv64 cpu 相关测试
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

#include "riscv64/include/cpu.hpp"

TEST(Riscv64RegInfoBaseTest, ValueTest) {
  EXPECT_EQ(cpu::reginfo::RegInfoBase::kBitOffset, 0);
  EXPECT_EQ(cpu::reginfo::RegInfoBase::kBitWidth, 64);
  EXPECT_EQ(cpu::reginfo::RegInfoBase::kBitMask, 0xFFFFFFFFFFFFFFFF);
  EXPECT_EQ(cpu::reginfo::RegInfoBase::kAllSetMask, 0xFFFFFFFFFFFFFFFF);
}

TEST(Riscv64FpInfoTest, ValueTest) {
  EXPECT_EQ(cpu::reginfo::FpInfo::kBitOffset, 0);
  EXPECT_EQ(cpu::reginfo::FpInfo::kBitWidth, 64);
  EXPECT_EQ(cpu::reginfo::FpInfo::kBitMask, 0xFFFFFFFFFFFFFFFF);
  EXPECT_EQ(cpu::reginfo::FpInfo::kAllSetMask, 0xFFFFFFFFFFFFFFFF);
}

TEST(Riscv64SstatusInfoSieTest, ValueTest) {
  EXPECT_EQ(cpu::reginfo::csr::SstatusInfo::Sie::kBitOffset, 1);
  EXPECT_EQ(cpu::reginfo::csr::SstatusInfo::Sie::kBitWidth, 1);
  EXPECT_EQ(cpu::reginfo::csr::SstatusInfo::Sie::kBitMask, 2);
  EXPECT_EQ(cpu::reginfo::csr::SstatusInfo::Sie::kAllSetMask, 1);
}

TEST(Riscv64SstatusInfoSpieTest, ValueTest) {
  EXPECT_EQ(cpu::reginfo::csr::SstatusInfo::Spie::kBitOffset, 5);
  EXPECT_EQ(cpu::reginfo::csr::SstatusInfo::Spie::kBitWidth, 1);
  EXPECT_EQ(cpu::reginfo::csr::SstatusInfo::Spie::kBitMask, 0x20);
  EXPECT_EQ(cpu::reginfo::csr::SstatusInfo::Spie::kAllSetMask, 1);
}

TEST(Riscv64SstatusInfoSppTest, ValueTest) {
  EXPECT_EQ(cpu::reginfo::csr::SstatusInfo::Spp::kBitOffset, 8);
  EXPECT_EQ(cpu::reginfo::csr::SstatusInfo::Spp::kBitWidth, 1);
  EXPECT_EQ(cpu::reginfo::csr::SstatusInfo::Spp::kBitMask, 0x100);
  EXPECT_EQ(cpu::reginfo::csr::SstatusInfo::Spp::kAllSetMask, 1);
}

TEST(Riscv64StvecInfoModeTest, ValueTest) {
  EXPECT_EQ(cpu::reginfo::csr::StvecInfo::Mode::kBitOffset, 0);
  EXPECT_EQ(cpu::reginfo::csr::StvecInfo::Mode::kBitWidth, 2);
  EXPECT_EQ(cpu::reginfo::csr::StvecInfo::Mode::kBitMask, 0x3);
  EXPECT_EQ(cpu::reginfo::csr::StvecInfo::Mode::kAllSetMask, 0x3);
}

TEST(Riscv64SipInfoSsipTest, ValueTest) {
  EXPECT_EQ(cpu::reginfo::csr::SipInfo::Ssip::kBitOffset, 1);
  EXPECT_EQ(cpu::reginfo::csr::SipInfo::Ssip::kBitWidth, 1);
  EXPECT_EQ(cpu::reginfo::csr::SipInfo::Ssip::kBitMask, 0x2);
  EXPECT_EQ(cpu::reginfo::csr::SipInfo::Ssip::kAllSetMask, 1);
}

TEST(Riscv64SipInfoStipTest, ValueTest) {
  EXPECT_EQ(cpu::reginfo::csr::SipInfo::Stip::kBitOffset, 05);
  EXPECT_EQ(cpu::reginfo::csr::SipInfo::Stip::kBitWidth, 1);
  EXPECT_EQ(cpu::reginfo::csr::SipInfo::Stip::kBitMask, 0x20);
  EXPECT_EQ(cpu::reginfo::csr::SipInfo::Stip::kAllSetMask, 1);
}

TEST(Riscv64SipInfoSeipTest, ValueTest) {
  EXPECT_EQ(cpu::reginfo::csr::SipInfo::Seip::kBitOffset, 9);
  EXPECT_EQ(cpu::reginfo::csr::SipInfo::Seip::kBitWidth, 1);
  EXPECT_EQ(cpu::reginfo::csr::SipInfo::Seip::kBitMask, 0x200);
  EXPECT_EQ(cpu::reginfo::csr::SipInfo::Seip::kAllSetMask, 1);
}

TEST(Riscv64SieInfoSsiepTest, ValueTest) {
  EXPECT_EQ(cpu::reginfo::csr::SieInfo::Ssie::kBitOffset, 1);
  EXPECT_EQ(cpu::reginfo::csr::SieInfo::Ssie::kBitWidth, 1);
  EXPECT_EQ(cpu::reginfo::csr::SieInfo::Ssie::kBitMask, 0x2);
  EXPECT_EQ(cpu::reginfo::csr::SieInfo::Ssie::kAllSetMask, 1);
}

TEST(Riscv64SieInfoStieTest, ValueTest) {
  EXPECT_EQ(cpu::reginfo::csr::SieInfo::Stie::kBitOffset, 5);
  EXPECT_EQ(cpu::reginfo::csr::SieInfo::Stie::kBitWidth, 1);
  EXPECT_EQ(cpu::reginfo::csr::SieInfo::Stie::kBitMask, 0x20);
  EXPECT_EQ(cpu::reginfo::csr::SieInfo::Stie::kAllSetMask, 1);
}

TEST(Riscv64SieInfoSeieTest, ValueTest) {
  EXPECT_EQ(cpu::reginfo::csr::SieInfo::Seie::kBitOffset, 9);
  EXPECT_EQ(cpu::reginfo::csr::SieInfo::Seie::kBitWidth, 1);
  EXPECT_EQ(cpu::reginfo::csr::SieInfo::Seie::kBitMask, 0x200);
  EXPECT_EQ(cpu::reginfo::csr::SieInfo::Seie::kAllSetMask, 1);
}
