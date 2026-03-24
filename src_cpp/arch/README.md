# arch

架构相关代码目录，包含了不同 CPU 架构的特定实现，目前支持两种主流 64 位架构：

- **aarch64** - ARM 64 位架构
- **riscv64** - RISC-V 64 位架构

## 目录结构

```
arch/
├── arch.h              # 架构抽象接口定义
├── CMakeLists.txt      # 构建配置文件
├── README.md          # 本文档
├── aarch64/           # ARM 64 位架构实现
│   ├── arch_main.cpp  # 架构初始化主函数
│   ├── backtrace.cpp  # 调用栈回溯实现
│   ├── boot.S         # 启动汇编代码
│   ├── early_console.cpp # 早期调试输出
│   └── link.ld        # 链接脚本
└── riscv64/           # RISC-V 64 位架构实现
    ├── arch_main.cpp  # 架构初始化主函数
    ├── backtrace.cpp  # 调用栈回溯实现
    ├── boot.S         # 启动汇编代码
    ├── early_console.cpp # 早期调试输出
    ├── link.ld        # 链接脚本
    └── macro.S        # 汇编宏定义
```

## 核心接口

### arch.h

定义了所有架构需要实现的统一接口：

```cpp
// 架构相关初始化
void ArchInit(int argc, const char **argv);
void ArchInitSMP(int argc, const char **argv);

// 调用栈回溯
int backtrace(void **buffer, int size);
void DumpStack();
```

## 各架构实现

### 通用功能

每个架构都实现了以下核心功能：

1. **启动引导** (`boot.S`)
   - CPU 初始化和寄存器设置
   - 栈空间分配和设置
   - 跳转到 C/C++ 入口点

2. **架构初始化** (`arch_main.cpp`)
   - 基础硬件初始化
   - 串口/控制台输出设置
   - 多核处理器启动
   - 系统信息收集

3. **调用栈回溯** (`backtrace.cpp`)
   - 基于帧指针的栈回溯
   - 符号表解析和函数名显示
   - 错误调试支持

4. **链接配置** (`link.ld`)
   - 内存布局定义
   - 段分配和对齐
   - 符号导出

### 架构特定功能

#### aarch64
- **启动方式**: 通过 ARM Trusted Firmware 或 U-Boot 启动
- **多核启动**: 使用 PSCI (Power State Coordination Interface) 唤醒其他核心
- **早期控制台**: 通过 SIMPLEKERNEL_EARLY_CONSOLE 预设支持早期调试输出 (PL011)
- **设备树**: 通过 FDT (Flattened Device Tree) 获取硬件信息
- **帧指针**: 使用 X29 寄存器进行栈回溯

#### riscv64
- **启动方式**: 通过 OpenSBI 启动
- **多核启动**: 使用 SBI (Supervisor Binary Interface) hart_start 调用
- **早期控制台**: 通过 SIMPLEKERNEL_EARLY_CONSOLE 预设支持早期调试输出 (SBI)
- **设备树**: 通过 FDT 获取硬件信息
- **帧指针**: 使用 FP 寄存器进行栈回溯
- **特殊文件**: `macro.S` 提供汇编宏定义，包括寄存器操作宏

## 编译配置

通过 CMake 根据 `CMAKE_SYSTEM_PROCESSOR` 变量自动选择对应的架构实现：

- **源文件**: 每个架构的 `boot.S`, `arch_main.cpp`, `backtrace.cpp`
- **头文件路径**: 自动包含对应架构的头文件目录
- **特殊处理**: RISC-V 架构额外包含 `macro.S` 文件

## 多核支持

所有架构都支持 SMP (Symmetric Multi-Processing)：

- **主核初始化**: `ArchInit()` 在主核上执行完整初始化
- **从核初始化**: `ArchInitSMP()` 在从核上执行简化初始化
- **核心启动**: 每个架构使用不同的机制唤醒其他 CPU 核心
- **栈管理**: 为每个核心分配独立的栈空间

## 调试支持

提供统一的调用栈回溯功能：

- **符号解析**: 通过解析内核 ELF 文件获取函数符号
- **地址映射**: 将运行时地址映射到函数名
- **错误定位**: 在内核崩溃时显示详细的调用栈信息

这种架构设计确保了内核的可移植性，同时允许每个架构充分利用其特有的硬件特性。
