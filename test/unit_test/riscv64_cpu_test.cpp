
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

// #include <gmock/gmock.h>
// #include <gtest/gtest.h>

#include <type_traits>

#include "riscv64/include/cpu.hpp"

// // 模拟类
// class MockReadOnlyRegBase {
//  public:
//   MOCK_METHOD(uint64_t, Read, (), (const, static));
//   MOCK_METHOD(uint64_t, CallOperator, (), (const, static));
// };

// // 模拟静态方法的行为
// uint64_t MockReadOnlyRegBase::Read() {
//   return -1;  // 默认行为
// }

// uint64_t MockReadOnlyRegBase::CallOperator() { return Read(); }

// // 包装类，用于在测试中替换静态方法的行为
// template <class Reg>
// class ReadOnlyRegBaseWrapper : public ReadOnlyRegBase<Reg> {
//  public:
//   static uint64_t Read() { return MockReadOnlyRegBase::Read(); }

//   static uint64_t operator()() { return MockReadOnlyRegBase::CallOperator();
//   }
// };

// using namespace testing;

// // 测试用例
// TEST(ReadOnlyRegBaseTest, ReadFpInfo) {
//   // 设置期望行为
//   EXPECT_CALL(MockReadOnlyRegBase(), Read()).WillOnce(Return(12345));

//   // 使用包装类进行测试
//   uint64_t value = ReadOnlyRegBaseWrapper<reginfo::FpInfo>::Read();
//   EXPECT_EQ(value, 12345);

//   value = ReadOnlyRegBaseWrapper<reginfo::FpInfo>()();
//   EXPECT_EQ(value, 12345);
// }

// int main(int argc, char **argv) {
//   ::testing::InitGoogleMock(&argc, argv);
//   return RUN_ALL_TESTS();
// }

int main() { return 0; }
