/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include <cpu_io.h>

#include <cstdint>

#include "arch.h"
#include "basic_info.hpp"
#include "kernel.h"
#include "kstd_cstring"
#include "kstd_libcxx.h"
#include "system_test.h"

template <uint32_t V>
class TestStaticConstructDestruct {
 public:
  explicit TestStaticConstructDestruct(unsigned int& v) : _v(v) { _v |= V; }
  ~TestStaticConstructDestruct() { _v &= ~V; }

 private:
  unsigned int& _v;
};

static int global_value_with_init = 42;
static uint32_t global_u32_value_with_init{0xa1a2a3a4UL};
static uint64_t global_u64_value_with_init{0xb1b2b3b4b5b6b7b8ULL};
static uint16_t global_u16_value_with_init{0x1234};
static uint8_t global_u8a_value_with_init{0x42};
static uint8_t global_u8b_value_with_init{0x43};
static uint8_t global_u8c_value_with_init{0x44};
static uint8_t global_u8d_value_with_init{0x45};
static volatile bool global_bool_keep_running{true};

static unsigned int global_value1_with_constructor = 1;
static unsigned int global_value2_with_constructor = 2;

static TestStaticConstructDestruct<0x200> constructor_destructor_1(
    global_value1_with_constructor);
static TestStaticConstructDestruct<0x200> constructor_destructor_2(
    global_value2_with_constructor);
static TestStaticConstructDestruct<0x100000> constructor_destructor_3{
    global_value2_with_constructor};
static TestStaticConstructDestruct<0x100000> constructor_destructor_4{
    global_value1_with_constructor};

static int g_dtor_count = 0;

class AbsClass {
 public:
  AbsClass() { val = 'B'; }
  virtual ~AbsClass() { g_dtor_count++; }
  virtual void Func() = 0;
  char val = 'A';
};

class InsClass : public AbsClass {
 public:
  void Func() override { val = 'C'; }
};

auto ctor_dtor_test() -> bool {
#ifdef __aarch64__
  cpu_io::SetupFpu();
#endif

  klog::Info("Running C++ Runtime Tests...");

  // 1. Verify Global Initialization
  EXPECT_EQ(global_value_with_init, 42, "Global int init");
  EXPECT_EQ(global_u32_value_with_init, 0xa1a2a3a4UL, "Global uint32 init");
  EXPECT_EQ(global_u64_value_with_init, 0xb1b2b3b4b5b6b7b8ULL,
            "Global uint64 init");
  EXPECT_EQ(global_u16_value_with_init, 0x1234, "Global uint16 init");
  EXPECT_EQ(global_u8a_value_with_init, 0x42, "Global uint8 a init");
  EXPECT_EQ(global_u8b_value_with_init, 0x43, "Global uint8 b init");
  EXPECT_EQ(global_u8c_value_with_init, 0x44, "Global uint8 c init");
  EXPECT_EQ(global_u8d_value_with_init, 0x45, "Global uint8 d init");

  // 2. Verify Global Constructors
  // global_value1_with_constructor was initialized to 1
  // constructor_destructor_1 adds 0x200
  // constructor_destructor_4 adds 0x100000
  // Expected: 1 | 0x200 | 0x100000 = 0x100201 (1049089)
  unsigned int expected_v1 = 1 | 0x200 | 0x100000;
  EXPECT_EQ(global_value1_with_constructor, expected_v1,
            "Global constructor execution 1");

  unsigned int expected_v2 = 2 | 0x200 | 0x100000;
  EXPECT_EQ(global_value2_with_constructor, expected_v2,
            "Global constructor execution 2");

  // 3. Verify Class Member, Virtual Function, and Stack Object Destructor
  int start_dtor = g_dtor_count;
  {
    auto inst_class = InsClass();
    EXPECT_EQ(inst_class.val, 'B', "Class constructor body");
    inst_class.Func();
    EXPECT_EQ(inst_class.val, 'C', "Virtual function dispatch");
  }
  EXPECT_EQ(g_dtor_count, start_dtor + 1,
            "Stack object destructor verification");

  // 4. Verify Static Local Variable
  static InsClass inst_class_static;
  EXPECT_TRUE(inst_class_static.val == 'B' || inst_class_static.val == 'C',
              "Static local object constructor/persistence");
  inst_class_static.Func();
  EXPECT_EQ(inst_class_static.val, 'C', "Static local object virtual func");

  klog::Info("PASS: All C++ Runtime Tests passed.");

  return true;
}
