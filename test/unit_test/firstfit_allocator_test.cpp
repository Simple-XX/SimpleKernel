
/**
 * @file firstfit_allocator_test.cpp
 * @brief firstfit_allocator 相关测试
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

#include "memory/firstfit_allocator.h"

#include <gtest/gtest.h>

TEST(FirstFitAllocatorTest, AllocAtTest) {
  auto allocator = FirstFitAllocator("AllocAtTest", 0, 0x1000);
  EXPECT_EQ(allocator.AllocAt(0, 0x1001), 0);
  EXPECT_EQ(allocator.AllocAt(0, 0x1000), 1);
  EXPECT_EQ(allocator.AllocAt(0, 0x1000), 0);
  allocator.Free(0, 0x1000);
  EXPECT_EQ(allocator.AllocAt(0, 0xFFF), 1);
  EXPECT_EQ(allocator.AllocAt(0x1000, 1), 0);
}

TEST(FirstFitAllocatorTest, AllocTest) {
  auto allocator = FirstFitAllocator("AllocTest", 0x1000, 0x1000);
  auto addr1 = allocator.Alloc(0x1001);
  EXPECT_EQ(addr1, 0);

  auto addr2 = allocator.Alloc(0x1000);
  EXPECT_EQ(addr2, 0x1000);

  auto addr3 = allocator.Alloc(0x1000);
  EXPECT_EQ(addr3, 0);

  allocator.Free(addr2, 0x1000);

  auto addr4 = allocator.Alloc(0xFFF);
  EXPECT_EQ(addr4, 0x1000);

  auto addr5 = allocator.Alloc(1);
  EXPECT_EQ(addr5, 0x1000 + 0xFFF * 0x1000);
}
