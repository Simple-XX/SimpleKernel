/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <gtest/gtest.h>

#include <cstring>

#include "io_buffer.hpp"

TEST(DmaRegionTest, DefaultConstructedIsInvalid) {
  DmaRegion region{};
  EXPECT_FALSE(region.IsValid());
  EXPECT_EQ(region.virt, nullptr);
  EXPECT_EQ(region.phys, 0u);
  EXPECT_EQ(region.size, 0u);
}

TEST(DmaRegionTest, ValidRegion) {
  uint8_t buf[256]{};
  DmaRegion region{
      .virt = buf,
      .phys = 0x8000'0000,
      .size = sizeof(buf),
  };
  EXPECT_TRUE(region.IsValid());
  EXPECT_EQ(region.Data(), buf);
  EXPECT_EQ(region.phys, 0x8000'0000u);
  EXPECT_EQ(region.size, 256u);
}

TEST(DmaRegionTest, NullVirtIsInvalid) {
  DmaRegion region{.virt = nullptr, .phys = 0x1000, .size = 4096};
  EXPECT_FALSE(region.IsValid());
}

TEST(DmaRegionTest, ZeroSizeIsInvalid) {
  uint8_t buf[1]{};
  DmaRegion region{.virt = buf, .phys = 0x1000, .size = 0};
  EXPECT_FALSE(region.IsValid());
}

TEST(DmaRegionTest, SubRegionSuccess) {
  uint8_t buf[4096]{};
  DmaRegion region{
      .virt = buf,
      .phys = 0x1'0000,
      .size = 4096,
  };

  auto sub = region.SubRegion(256, 512);
  ASSERT_TRUE(sub.has_value());
  EXPECT_EQ(sub->virt, buf + 256);
  EXPECT_EQ(sub->phys, 0x1'0000u + 256);
  EXPECT_EQ(sub->size, 512u);
  EXPECT_TRUE(sub->IsValid());
}

TEST(DmaRegionTest, SubRegionOutOfBoundsFails) {
  uint8_t buf[256]{};
  DmaRegion region{.virt = buf, .phys = 0x1000, .size = 256};

  auto sub = region.SubRegion(200, 100);  // 200 + 100 > 256
  EXPECT_FALSE(sub.has_value());
  EXPECT_EQ(sub.error().code, ErrorCode::kInvalidArgument);
}

TEST(DmaRegionTest, SubRegionExactBoundarySucceeds) {
  uint8_t buf[256]{};
  DmaRegion region{.virt = buf, .phys = 0x1000, .size = 256};

  auto sub = region.SubRegion(0, 256);  // exact fit
  ASSERT_TRUE(sub.has_value());
  EXPECT_EQ(sub->size, 256u);
}

TEST(DmaRegionTest, IdentityVirtToPhys) {
  uintptr_t addr = 0xDEAD'BEEF;
  EXPECT_EQ(IdentityVirtToPhys(addr), addr);
}
