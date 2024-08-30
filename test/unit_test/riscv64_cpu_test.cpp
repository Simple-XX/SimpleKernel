
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

#include "riscv64/include/cpu/cpu.hpp"

TEST(Riscv64RegInfoBaseTest, ValueTest) {
  EXPECT_EQ(cpu::register_info::RegInfoBase::kBitOffset, 0);
  EXPECT_EQ(cpu::register_info::RegInfoBase::kBitWidth, 64);
  EXPECT_EQ(cpu::register_info::RegInfoBase::kBitMask, 0xFFFFFFFFFFFFFFFF);
  EXPECT_EQ(cpu::register_info::RegInfoBase::kAllSetMask, 0xFFFFFFFFFFFFFFFF);
}

TEST(Riscv64FpInfoTest, ValueTest) {
  EXPECT_EQ(cpu::register_info::FpInfo::kBitOffset, 0);
  EXPECT_EQ(cpu::register_info::FpInfo::kBitWidth, 64);
  EXPECT_EQ(cpu::register_info::FpInfo::kBitMask, 0xFFFFFFFFFFFFFFFF);
  EXPECT_EQ(cpu::register_info::FpInfo::kAllSetMask, 0xFFFFFFFFFFFFFFFF);
}

TEST(Riscv64SstatusInfoSieTest, ValueTest) {
  EXPECT_EQ(cpu::register_info::csr::SstatusInfo::Sie::kBitOffset, 1);
  EXPECT_EQ(cpu::register_info::csr::SstatusInfo::Sie::kBitWidth, 1);
  EXPECT_EQ(cpu::register_info::csr::SstatusInfo::Sie::kBitMask, 2);
  EXPECT_EQ(cpu::register_info::csr::SstatusInfo::Sie::kAllSetMask, 1);
}

TEST(Riscv64SstatusInfoSpieTest, ValueTest) {
  EXPECT_EQ(cpu::register_info::csr::SstatusInfo::Spie::kBitOffset, 5);
  EXPECT_EQ(cpu::register_info::csr::SstatusInfo::Spie::kBitWidth, 1);
  EXPECT_EQ(cpu::register_info::csr::SstatusInfo::Spie::kBitMask, 0x20);
  EXPECT_EQ(cpu::register_info::csr::SstatusInfo::Spie::kAllSetMask, 1);
}

TEST(Riscv64SstatusInfoSppTest, ValueTest) {
  EXPECT_EQ(cpu::register_info::csr::SstatusInfo::Spp::kBitOffset, 8);
  EXPECT_EQ(cpu::register_info::csr::SstatusInfo::Spp::kBitWidth, 1);
  EXPECT_EQ(cpu::register_info::csr::SstatusInfo::Spp::kBitMask, 0x100);
  EXPECT_EQ(cpu::register_info::csr::SstatusInfo::Spp::kAllSetMask, 1);
}

TEST(Riscv64StvecInfoModeTest, ValueTest) {
  EXPECT_EQ(cpu::register_info::csr::StvecInfo::Mode::kBitOffset, 0);
  EXPECT_EQ(cpu::register_info::csr::StvecInfo::Mode::kBitWidth, 2);
  EXPECT_EQ(cpu::register_info::csr::StvecInfo::Mode::kBitMask, 0x3);
  EXPECT_EQ(cpu::register_info::csr::StvecInfo::Mode::kAllSetMask, 0x3);
}

TEST(Riscv64SipInfoSsipTest, ValueTest) {
  EXPECT_EQ(cpu::register_info::csr::SipInfo::Ssip::kBitOffset, 1);
  EXPECT_EQ(cpu::register_info::csr::SipInfo::Ssip::kBitWidth, 1);
  EXPECT_EQ(cpu::register_info::csr::SipInfo::Ssip::kBitMask, 0x2);
  EXPECT_EQ(cpu::register_info::csr::SipInfo::Ssip::kAllSetMask, 1);
}

TEST(Riscv64SipInfoStipTest, ValueTest) {
  EXPECT_EQ(cpu::register_info::csr::SipInfo::Stip::kBitOffset, 05);
  EXPECT_EQ(cpu::register_info::csr::SipInfo::Stip::kBitWidth, 1);
  EXPECT_EQ(cpu::register_info::csr::SipInfo::Stip::kBitMask, 0x20);
  EXPECT_EQ(cpu::register_info::csr::SipInfo::Stip::kAllSetMask, 1);
}

TEST(Riscv64SipInfoSeipTest, ValueTest) {
  EXPECT_EQ(cpu::register_info::csr::SipInfo::Seip::kBitOffset, 9);
  EXPECT_EQ(cpu::register_info::csr::SipInfo::Seip::kBitWidth, 1);
  EXPECT_EQ(cpu::register_info::csr::SipInfo::Seip::kBitMask, 0x200);
  EXPECT_EQ(cpu::register_info::csr::SipInfo::Seip::kAllSetMask, 1);
}

TEST(Riscv64SieInfoSsiepTest, ValueTest) {
  EXPECT_EQ(cpu::register_info::csr::SieInfo::Ssie::kBitOffset, 1);
  EXPECT_EQ(cpu::register_info::csr::SieInfo::Ssie::kBitWidth, 1);
  EXPECT_EQ(cpu::register_info::csr::SieInfo::Ssie::kBitMask, 0x2);
  EXPECT_EQ(cpu::register_info::csr::SieInfo::Ssie::kAllSetMask, 1);
}

TEST(Riscv64SieInfoStieTest, ValueTest) {
  EXPECT_EQ(cpu::register_info::csr::SieInfo::Stie::kBitOffset, 5);
  EXPECT_EQ(cpu::register_info::csr::SieInfo::Stie::kBitWidth, 1);
  EXPECT_EQ(cpu::register_info::csr::SieInfo::Stie::kBitMask, 0x20);
  EXPECT_EQ(cpu::register_info::csr::SieInfo::Stie::kAllSetMask, 1);
}

TEST(Riscv64SieInfoSeieTest, ValueTest) {
  EXPECT_EQ(cpu::register_info::csr::SieInfo::Seie::kBitOffset, 9);
  EXPECT_EQ(cpu::register_info::csr::SieInfo::Seie::kBitWidth, 1);
  EXPECT_EQ(cpu::register_info::csr::SieInfo::Seie::kBitMask, 0x200);
  EXPECT_EQ(cpu::register_info::csr::SieInfo::Seie::kAllSetMask, 1);
}

TEST(Riscv64ScauseInfoExceptionCodeTest, ValueTest) {
  EXPECT_EQ(cpu::register_info::csr::ScauseInfo::ExceptionCode::kBitOffset, 0);
  EXPECT_EQ(cpu::register_info::csr::ScauseInfo::ExceptionCode::kBitWidth, 63);
  EXPECT_EQ(cpu::register_info::csr::ScauseInfo::ExceptionCode::kBitMask,
            0x7FFFFFFFFFFFFFFF);
  EXPECT_EQ(cpu::register_info::csr::ScauseInfo::ExceptionCode::kAllSetMask,
            0x7FFFFFFFFFFFFFFF);
}

TEST(Riscv64ScauseInfoInterruptTest, ValueTest) {
  EXPECT_EQ(cpu::register_info::csr::ScauseInfo::Interrupt::kBitOffset, 63);
  EXPECT_EQ(cpu::register_info::csr::ScauseInfo::Interrupt::kBitWidth, 1);
  EXPECT_EQ(cpu::register_info::csr::ScauseInfo::Interrupt::kBitMask,
            0x8000000000000000);
  EXPECT_EQ(cpu::register_info::csr::ScauseInfo::Interrupt::kAllSetMask, 1);
}

TEST(Riscv64SatpInfoPpnTest, ValueTest) {
  EXPECT_EQ(cpu::register_info::csr::SatpInfo::Ppn::kBitOffset, 0);
  EXPECT_EQ(cpu::register_info::csr::SatpInfo::Ppn::kBitWidth, 44);
  EXPECT_EQ(cpu::register_info::csr::SatpInfo::Ppn::kBitMask, 0xFFFFFFFFFFF);
  EXPECT_EQ(cpu::register_info::csr::SatpInfo::Ppn::kAllSetMask, 0xFFFFFFFFFFF);
}

TEST(Riscv64SatpInfoAsidTest, ValueTest) {
  EXPECT_EQ(cpu::register_info::csr::SatpInfo::Asid::kBitOffset, 44);
  EXPECT_EQ(cpu::register_info::csr::SatpInfo::Asid::kBitWidth, 16);
  EXPECT_EQ(cpu::register_info::csr::SatpInfo::Asid::kBitMask,
            ~0xF0000FFFFFFFFFFF);
  EXPECT_EQ(cpu::register_info::csr::SatpInfo::Asid::kAllSetMask, 0xFFFF);
}

TEST(Riscv64SatpInfoModeTest, ValueTest) {
  EXPECT_EQ(cpu::register_info::csr::SatpInfo::Mode::kBitOffset, 60);
  EXPECT_EQ(cpu::register_info::csr::SatpInfo::Mode::kBitWidth, 4);
  EXPECT_EQ(cpu::register_info::csr::SatpInfo::Mode::kBitMask,
            ~0xFFFFFFFFFFFFFFF);
  EXPECT_EQ(cpu::register_info::csr::SatpInfo::Mode::kAllSetMask, 0xF);
}
