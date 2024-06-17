
/**
 * @file kernel_fdt_test.cpp
 * @brief fdt 解析测试
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

#include "kernel_fdt.hpp"

#include <gtest/gtest.h>

#include "riscv64_virt.dtb.h"

TEST(KernelFdtTest, ConstructorTest) {
  KernelFdt kerlen_fdt((uint64_t)riscv64_virt_dtb_data);
}

TEST(KernelFdtTest, GetMemoryTest) {
  KernelFdt kerlen_fdt((uint64_t)riscv64_virt_dtb_data);

  auto [memory_base, memory_size] = kKernelFdt.GetInstance().GetMemory();

  EXPECT_EQ(memory_base, 0x80000000);
  EXPECT_EQ(memory_size, 0x8000000);
}

TEST(KernelFdtTest, GetSerialTest) {
  KernelFdt kerlen_fdt((uint64_t)riscv64_virt_dtb_data);

  auto [serial_base, serial_size] = kKernelFdt.GetInstance().GetSerial();

  EXPECT_EQ(serial_base, 0x10000000);
  EXPECT_EQ(serial_size, 0x100);
}
