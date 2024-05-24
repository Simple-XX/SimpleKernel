
# fix_FFFF

此版本用于讨论bug

https://github.com/MRNIU/SimpleKernel/tree/fix_FFFF

## How To Use（需要科学上网）

- 安装依赖(Ubuntu) 

  ```shell
  sudo apt install --fix-missing -y doxygen graphviz clang-format clang-tidy cppcheck qemu-system lcov gdb-multiarch libgtest-dev cmake
  sudo apt install --fix-missing -y gcc-riscv64-linux-gnu g++-riscv64-linux-gnu
  ```

- 编译

  ```shell
  cd SimpleKernel
  cmake --preset build_riscv64
  cd build_riscv64
  make kernel
  ```

- 运行(于上一步的 build_riscv64 目录下)

  ```shell
  make run_run
  ```

- 调试(于上一步的 build_riscv64 目录下)

  ```shell
  make run_debug
  ```

  此时需要新开一个 shell

  ```shell
  gdb-multiarch
  ```

  进入 gdb 界面后输入

  ```shell
  target remote :1234
  file image/kernel.elf
  # 打断点到 main
  b main
  # 开始运行
  c
  ```

- 可能有用的指令

  ```shell
  # 在 qemu 中运行
  make run_run
  # 以 debug 模式在 qemu 中运行，可以使用 gdb 连接
  make run_debug
  # 编译 kernel，同时生成 objdump 与 readelf 结果
  make kernel
  ```


## 问题描述

riscv64 静态全局对象的地址与 readelf 不一致，这导致在手动进行 c++ 全局对象初始化的时候会访问到非法内存。

```c++
class aaa {
 public:
  int a = 233;

  aaa() : a(666) { printf("aaa init\n"); }

  aaa(int _a) : a(_a) { printf("aaa init %d\n", _a); }
};

// auto class_a = aaa(2);
// static aaa class_a2 = aaa(3);
int i32;
static int si32;

int main(int _argc, char** _argv) {
  // 架构相关初始化
  auto arch_init_ret = arch_init(_argc, reinterpret_cast<uint8_t**>(_argv));

  // printf("class_a.a: %d\n", class_a.a);
  // printf("&class_a: %p\n", &class_a);
  // printf("&class_a2: %p\n", &class_a2);
  printf("&i32: %p\n", &i32);
  printf("&si32: %p\n", &si32);
  printf("*(&si32): %p\n", *(&si32));
  uintptr_t ccc = 0;
  asm("nop");
  ccc = (uintptr_t)&si32;
  asm("nop");

  printf("ccc: %X\n", ccc);

  printf("------\n");

  // 进入死循环
  while (1) {
    ;
  }
  return 0;
}
```

通过查看汇编发现，使用了 gp 寄存器

```asm
    802140b0:	0001                	nop
    802140b2:	93418793          	add	a5,gp,-1740 # 8021b134 <_ZL4si32>
    802140b6:	fef43023          	sd	a5,-32(s0)
    802140ba:	0001                	nop
```

解决：添加 -mno-relax 参数

参考：https://stackoverflow.com/questions/67505060


1. 在 gdb 中 `p &si32`，结果符合预期(地址为 8021XXXX，与 readelf 一致)
2. 在程序中 `ccc = &si32`，然后在 gdb 中 `p ccc`，结果错误(地址为 FFFFFFFFFFFFFXXX)

## 相关文件路径

编译参数 cmake/compile_config.cmake

链接脚本 src/kernel/arch/riscv64/link.ld

boot src/kernel/arch/riscv64/boot.S

cpp 初始化 src/kernel/libcxx/libcxx.cpp

内核入口 src/kernel/main.cpp

反汇编文件 build_riscv64/src/kernel/kernel.elf.disassembly

readelf 文件 build_riscv64/src/kernel/kernel.elf.readelf
