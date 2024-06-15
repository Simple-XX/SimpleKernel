
/**
 * @file riscv_cpu_test.cpp
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdint>

#include "riscv64/include/cpu.hpp"

class ReadOnlyRegBaseInterface {
 public:
  virtual ~ReadOnlyRegBaseInterface() = default;
  virtual uint64_t Read() = 0;
};

template <class Reg>
class ReadOnlyRegBaseInterfaceWrapper : public ReadOnlyRegBaseInterface {
 public:
  uint64_t Read() override { return cpu::ReadOnlyRegBase<Reg>::Read(); }
};

class MockReadOnlyRegBase : public ReadOnlyRegBaseInterface {
 public:
  MOCK_METHOD(uint64_t, Read, (), (override));
};

using ::testing::Return;

TEST(StaticClassTest, StaticMethodTest) {
  MockReadOnlyRegBase mock;
  EXPECT_CALL(mock, Read()).WillOnce(Return(10));

  ReadOnlyRegBaseInterface* instance = &mock;
  EXPECT_EQ(instance->Read(), 10);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
