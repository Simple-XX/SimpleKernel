[![codecov](https://codecov.io/gh/Simple-XX/SimpleKernel/graph/badge.svg?token=J7NKK3SBNJ)](https://codecov.io/gh/Simple-XX/SimpleKernel)
![workflow](https://github.com/Simple-XX/SimpleKernel/actions/workflows/workflow.yml/badge.svg)
![commit-activity](https://img.shields.io/github/commit-activity/t/Simple-XX/SimpleKernel)
![last-commit-interrupt](https://img.shields.io/github/last-commit/Simple-XX/SimpleKernel/main)
![MIT License](https://img.shields.io/github/license/mashape/apistatus.svg)
[![LICENSE](https://img.shields.io/badge/license-Anti%20996-blue.svg)](https://github.com/996icu/996.ICU/blob/master/LICENSE)
[![996.icu](https://img.shields.io/badge/link-996.icu-red.svg)](https://996.icu)

[English](./README_ENG.md) | [中文](./README.md)

# SimpleKernel

**面向 AI 的操作系统学习项目 | Interface-Driven OS Kernel for AI-Assisted Learning**

> 🤖 **设计理念**：定义清晰的内核接口，由 AI 完成实现——学习操作系统的新范式

## 📖 目录

- [✨ 项目简介](#-项目简介)
- [🤖 面向 AI 的设计理念](#-面向-ai-的设计理念)
- [🏛️ 接口体系总览](#️-接口体系总览)
- [🏗️ 支持架构](#️-支持架构)
- [🚀 快速开始](#-快速开始)
- [📂 项目结构](#-项目结构)
- [🎯 学习路线](#-学习路线)
- [📦 第三方依赖](#-第三方依赖)
- [📝 开发指南](#-开发指南)
- [🤝 贡献指南](#-贡献指南)
- [📄 许可证](#-许可证)

## ✨ 项目简介

SimpleKernel 是一个**面向 AI 辅助学习的现代化操作系统内核项目**。采用 C++23 编写，支持 x86_64、RISC-V 64 和 AArch64 三种架构。

与传统 OS 教学项目不同，SimpleKernel 采用**接口驱动（Interface-Driven）** 的设计：

- **项目主体是接口定义**——完整的头文件（`.h/.hpp`）包含类声明、纯虚接口、类型定义、Doxygen 文档
- **实现由 AI 完成**——你只需要理解接口契约，让 AI 根据接口文档生成实现代码
- **参考实现可供对照**——项目提供完整的参考实现，用于验证 AI 生成代码的正确性

### 🌟 核心亮点

| 特性 | 说明 |
|------|------|
| 🤖 **AI-First 设计** | 接口文档即 prompt，AI 可直接根据头文件生成完整实现 |
| 📐 **接口与实现分离** | 头文件只有声明和契约，实现在独立的 `.cpp` 中 |
| 🌐 **三架构支持** | x86_64、RISC-V 64、AArch64，同一套接口适配不同硬件 |
| 🧪 **测试驱动验证** | GoogleTest 测试套件验证 AI 生成的实现是否符合接口契约 |
| 📖 **完整 Doxygen 文档** | 每个接口都有职责描述、前置条件、后置条件、使用示例 |
| 🏗️ **工程化基础设施** | CMake 构建、Dev Container 环境、CI/CD、clang-format/clang-tidy |

## 🤖 面向 AI 的设计理念

### 为什么要"面向 AI"？

传统 OS 教学项目的学习路径：**读代码 → 理解原理 → 模仿修改**。这种方式存在几个问题：

1. 内核代码量大，初学者容易迷失在实现细节中
2. 各模块耦合紧密，难以独立理解单个子系统
3. 从零实现一个模块的门槛很高，反馈周期长

SimpleKernel 提出一种新范式：**读接口 → 理解契约 → AI 实现 → 测试验证**

```
┌─────────────────────────────────────────────────────────┐
│                    SimpleKernel 学习流程                   │
│                                                         │
│   ┌──────────┐    ┌──────────┐    ┌──────────┐         │
│   │ 📐 接口   │───▶│ 🤖 AI    │───▶│ 🧪 测试  │         │
│   │ 头文件    │    │ 生成实现  │    │ 验证正确性│         │
│   │ + Doxygen │    │ (.cpp)   │    │ GoogleTest│         │
│   └──────────┘    └──────────┘    └──────────┘         │
│        │                               │                │
│        │         ┌──────────┐          │                │
│        └────────▶│ 📚 参考   │◀─────────┘                │
│                  │ 实现对照  │                           │
│                  └──────────┘                           │
└─────────────────────────────────────────────────────────┘
```

### 核心工作流

#### 1️⃣ 阅读接口，理解契约

每个模块的头文件都包含完整的接口文档：

```cpp
/**
 * @brief 中断子系统抽象基类
 *
 * 所有架构的中断处理必须实现此接口。
 *
 * @pre  硬件中断控制器已初始化
 * @post 可通过 RegisterInterruptFunc 注册中断处理函数
 *
 * 已知实现：PLIC（RISC-V）、GIC（AArch64）、APIC（x86_64）
 */
class InterruptBase {
public:
  virtual ~InterruptBase() = default;

  /// 执行中断处理
  virtual void Do(uint64_t cause, cpu_io::TrapContext* context) = 0;

  /// 注册中断处理函数
  virtual void RegisterInterruptFunc(uint64_t cause, InterruptFunc func) = 0;
};
```

#### 2️⃣ 让 AI 实现

将头文件作为上下文提供给 AI（如 GitHub Copilot、ChatGPT、Claude 等），要求其生成 `.cpp` 实现。接口的 Doxygen 注释就是最好的 prompt。

#### 3️⃣ 测试验证

运行项目自带的测试套件，验证 AI 生成的实现是否符合接口契约：

```shell
cmake --preset build_riscv64
cd build_riscv64 && make unit-test
```

#### 4️⃣ 对照参考实现

如果测试不通过，可以参考项目提供的参考实现进行对照和学习。

### 与 AI 工具的结合方式

| 场景 | 使用方式 |
|------|---------|
| **GitHub Copilot** | 打开头文件，在对应的 `.cpp` 中让 Copilot 自动补全实现 |
| **ChatGPT / Claude** | 将头文件内容粘贴为上下文，要求生成完整的 `.cpp` 实现 |
| **Copilot Chat / Cursor** | 在 IDE 中选中接口，要求 AI 解释契约含义或生成实现 |
| **自主学习** | 先独立思考实现思路，再让 AI 生成，对比差异 |

## 🏛️ 接口体系总览

SimpleKernel 的接口按功能分为以下层次：

```
┌──────────────────────────────────────────┐
│              应用/系统调用层               │
│         syscall.h · SyscallInit          │
├──────────────────────────────────────────┤
│               任务管理层                  │
│  TaskManager · SchedulerBase · Mutex     │
│  CfsScheduler · FifoScheduler · RR ...   │
├──────────────────────────────────────────┤
│               内存管理层                  │
│  VirtualMemory · PhysicalMemory          │
│  MapPage · UnmapPage · AllocFrame        │
├──────────────────────────────────────────┤
│               中断/异常层                 │
│  InterruptBase · RegisterInterruptFunc   │
│  TimerInit · InterruptInit               │
├──────────────────────────────────────────┤
│               设备框架层                    │
│  DeviceManager · DriverRegistry            │
│  PlatformBus · Ns16550aDriver · VirtioBlk  │
├──────────────────────────────────────────┤
│             架构抽象层 (arch.h)            │
│  ArchInit · InterruptInit · TimerInit    │
│  EarlyConsole（全局构造阶段自动设置）      │
├──────────────────────────────────────────┤
│            运行时支持库                    │
│  libc (sk_stdio.h, sk_string.h, ...)     │
│  libcxx (kstd_vector, __cxa_*, ...)      │
├──────────────────────────────────────────┤
│            硬件 / QEMU                    │
│  x86_64 · RISC-V 64 · AArch64           │
└──────────────────────────────────────────┘
```

### 关键接口文件

| 接口文件 | 职责 | 实现文件 |
|---------|------|---------|
| `src/arch/arch.h` | 架构无关的统一入口 | 各 `src/arch/{arch}/` 目录 |
| `src/include/interrupt_base.h` | 中断子系统抽象基类 | `src/arch/{arch}/interrupt.cpp` |
| `src/device/include/device_manager.hpp` | 设备管理器 | header-only |
| `src/device/include/driver_registry.hpp` | 驱动注册中心 | header-only |
| `src/device/include/platform_bus.hpp` | 平台总线（FDT 枚举） | header-only |
| `src/device/include/driver/ns16550a_driver.hpp` | NS16550A UART 驱动 | header-only（Probe/Remove 模式） |
| `src/include/virtual_memory.hpp` | 虚拟内存管理接口 | `src/virtual_memory.cpp` |
| `src/include/kernel_fdt.hpp` | 设备树解析接口 | `src/kernel_fdt.cpp` |
| `src/include/kernel_elf.hpp` | ELF 解析接口 | `src/kernel_elf.cpp` |
| `src/task/include/scheduler_base.hpp` | 调度器抽象基类 | `cfs_scheduler.cpp` 等 |
| `src/include/spinlock.hpp` | 自旋锁接口 | header-only（性能要求） |
| `src/include/mutex.hpp` | 互斥锁接口 | `src/task/mutex.cpp` |

> 📋 完整接口重构计划见 [docs/TODO_interface_refactor.md](./docs/TODO_interface_refactor.md)

## 🏗️ 支持架构

| 架构 | 引导链 | 串口 | 中断控制器 | 时钟 |
|:---:|:---:|:---:|:---:|:---:|
| **x86_64** | U-Boot | NS16550A | 8259A PIC | 8253/8254 |
| **RISC-V 64** | U-Boot + OpenSBI | SBI Call | Direct 模式 | SBI Timer |
| **AArch64** | U-Boot + ATF + OP-TEE | PL011 | GICv3 | Generic Timer |

## 🚀 快速开始

### 📋 系统要求

- **操作系统**: Linux (推荐 Ubuntu 24.04) 或 macOS
- **容器引擎**: Docker 或兼容的容器运行时
- **工具链**: 已包含在 Dev Container 中（GCC 14 交叉编译器、CMake、QEMU 等）
- **AI 工具（推荐）**: GitHub Copilot / ChatGPT / Claude

### 🛠️ 环境搭建

**方式一：使用 Dev Container（推荐）**

```shell
# 1. 克隆项目
git clone https://github.com/simple-xx/SimpleKernel.git
cd SimpleKernel

# 2. 使用 VS Code 打开并在容器中重新打开
#    安装 Dev Containers 扩展后，点击左下角 >< 图标
#    选择 "Reopen in Container"

# 或使用 CLI
npm install -g @devcontainers/cli
devcontainer up --workspace-folder .
devcontainer exec --workspace-folder . bash
```

> 也支持 **GitHub Codespaces**：点击仓库页面的 Code → Codespaces → Create codespace on main
>
> 详细说明见 [Dev Container 文档](./docs/docker.md)

**方式二：本地环境**

参考 [工具链文档](./docs/0_工具链.md) 配置本地开发环境。

### ⚡ 编译与运行

```shell
cd SimpleKernel

# 选择目标架构编译（以 RISC-V 64 为例）
cmake --preset build_riscv64
cd build_riscv64

# 编译内核
make SimpleKernel

# 在 QEMU 模拟器中运行
make run

# 运行单元测试（验证你的实现）
make unit-test
```

**支持的架构预设：**
- `build_riscv64` - RISC-V 64 位架构
- `build_aarch64` - ARM 64 位架构
- `build_x86_64` - x86 64 位架构

### 🎯 AI 辅助开发工作流

```shell
# 1. 在 VS Code 中打开项目（推荐安装 GitHub Copilot 扩展）
code ./SimpleKernel

# 2. 阅读头文件中的接口定义（例如 src/include/virtual_memory.hpp）

# 3. 创建/编辑对应的 .cpp 文件，让 AI 根据接口生成实现

# 4. 编译验证
cd build_riscv64 && make SimpleKernel

# 5. 运行测试
make unit-test

# 6. 在 QEMU 中运行，观察行为
make run
```

## 📂 项目结构

```
SimpleKernel/
├── src/                        # 内核源码
│   ├── include/                # 📐 公共接口头文件（项目核心）
│   │   ├── virtual_memory.hpp  #   虚拟内存管理接口
│   │   ├── kernel_fdt.hpp      #   设备树解析接口
│   │   ├── kernel_elf.hpp      #   ELF 解析接口
│   │   ├── spinlock.hpp        #   自旋锁接口
│   │   ├── mutex.hpp           #   互斥锁接口
│   │   └── ...
│   ├── arch/                   # 架构相关代码
│   │   ├── arch.h              # 📐 架构无关统一接口
│   │   ├── aarch64/            #   AArch64 实现
│   │   ├── riscv64/            #   RISC-V 64 实现
│   │   └── x86_64/             #   x86_64 实现
│   ├── device/                 # 设备管理框架
│   │   ├── include/            # 📐 设备框架接口（DeviceManager, DriverRegistry, Bus 等）
│   │   │   └── driver/         #   具体驱动（ns16550a_driver.hpp, virtio_blk_driver.hpp）
│   │   └── device.cpp          #   设备初始化入口（DeviceInit）
│   ├── task/                   # 任务管理
│   │   ├── include/            # 📐 调度器接口（SchedulerBase 等）
│   │   └── ...                 #   调度器实现
│   ├── libc/                   # 内核 C 标准库
│   └── libcxx/                 # 内核 C++ 运行时
├── tests/                      # 🧪 测试套件
│   ├── unit_test/              #   单元测试
│   ├── integration_test/       #   集成测试
│   └── system_test/            #   系统测试（QEMU 运行）
├── docs/                        # 📚 文档
│   ├── TODO_interface_refactor.md  # 接口重构计划
│   └── ...
├── cmake/                      # CMake 构建配置
├── 3rd/                        # 第三方依赖（Git Submodule）
└── tools/                      # 构建工具和模板
```

> 📐 标记的目录/文件是**接口定义**——这是你需要重点阅读的内容。

## 🎯 学习路线

建议按以下顺序学习和实现各模块：

### 阶段 1：基础设施（Boot）

| 模块 | 接口文件 | 难度 | 说明 |
|------|---------|:---:|------|
| Early Console | `src/arch/arch.h` 注释 | ⭐ | 最早期的输出，理解全局构造 |
| 串口驱动 | `ns16550a_driver.hpp` | ⭐⭐ | 实现 Probe/Remove，理解设备框架和 MMIO |
| 设备树解析 | `kernel_fdt.hpp` | ⭐⭐ | 解析硬件信息，理解 FDT 格式 |
| ELF 解析 | `kernel_elf.hpp` | ⭐⭐ | 符号表解析，用于栈回溯 |

### 阶段 2：中断系统（Interrupt）

| 模块 | 接口文件 | 难度 | 说明 |
|------|---------|:---:|------|
| 中断基类 | `interrupt_base.h` | ⭐⭐ | 理解中断处理的统一抽象 |
| 中断控制器 | 各架构驱动头文件 | ⭐⭐⭐ | GIC/PLIC/PIC 硬件编程 |
| 时钟中断 | `arch.h → TimerInit` | ⭐⭐ | 定时器配置，tick 驱动 |

### 阶段 3：内存管理（Memory）

| 模块 | 接口文件 | 难度 | 说明 |
|------|---------|:---:|------|
| 虚拟内存 | `virtual_memory.hpp` | ⭐⭐⭐ | 页表管理、地址映射 |
| 物理内存 | 相关接口 | ⭐⭐⭐ | 帧分配器、伙伴系统 |

### 阶段 4：任务管理（Thread/Task）

| 模块 | 接口文件 | 难度 | 说明 |
|------|---------|:---:|------|
| 自旋锁 | `spinlock.hpp` | ⭐⭐ | 原子操作，多核同步 |
| 互斥锁 | `mutex.hpp` | ⭐⭐⭐ | 基于任务阻塞的锁 |
| 调度器 | `scheduler_base.hpp` | ⭐⭐⭐ | CFS/FIFO/RR 调度算法 |

### 阶段 5：系统调用（Syscall）

| 模块 | 接口文件 | 难度 | 说明 |
|------|---------|:---:|------|
| 系统调用 | `arch.h → SyscallInit` | ⭐⭐⭐ | 用户态/内核态切换 |

## 📦 第三方依赖

| 依赖 | 用途 |
|------|------|
| [google/googletest](https://github.com/google/googletest.git) | 测试框架 |
| [charlesnicholson/nanoprintf](https://github.com/charlesnicholson/nanoprintf.git) | printf 实现 |
| [MRNIU/cpu_io](https://github.com/MRNIU/cpu_io.git) | CPU I/O 操作 |
| [riscv-software-src/opensbi](https://github.com/riscv-software-src/opensbi.git) | RISC-V SBI 实现 |
| [MRNIU/opensbi_interface](https://github.com/MRNIU/opensbi_interface.git) | OpenSBI 接口 |
| [u-boot/u-boot](https://github.com/u-boot/u-boot.git) | 通用引导程序 |
| [OP-TEE/optee_os](https://github.com/OP-TEE/optee_os.git) | OP-TEE 操作系统 |
| [ARM-software/arm-trusted-firmware](https://github.com/ARM-software/arm-trusted-firmware.git) | ARM 可信固件 |
| [dtc/dtc](https://git.kernel.org/pub/scm/utils/dtc/dtc.git) | 设备树编译器 |
| [MRNIU/bmalloc](https://github.com/MRNIU/bmalloc.git) | 内存分配器 |
| [MRNIU/MPMCQueue](https://github.com/MRNIU/MPMCQueue.git) | 无锁 MPMC 队列 |
| [MRNIU/device_framework](https://github.com/MRNIU/device_framework.git) | 设备管理框架 |

## 📝 开发指南

### 🎨 代码风格

- **语言标准**: C23 / C++23
- **编码规范**: [Google C++ Style Guide](https://zh-google-styleguide.readthedocs.io/en/latest/google-cpp-styleguide/contents.html)
- **自动格式化**: `.clang-format` + `.clang-tidy`
- **注释规范**: Doxygen 风格，接口文件必须包含完整的契约文档

### 命名约定

| 类型 | 风格 | 示例 |
|------|------|------|
| 文件 | 小写下划线 | `kernel_log.hpp` |
| 类/结构体 | PascalCase | `TaskManager` |
| 函数 | PascalCase / snake_case | `ArchInit` / `sys_yield` |
| 变量 | snake_case | `per_cpu_data` |
| 宏 | SCREAMING_SNAKE | `SIMPLEKERNEL_DEBUG` |
| 常量 | kCamelCase | `kPageSize` |
| 内核 libc/libc++ 头文件 | libc: `sk_` 前缀, libcxx: `kstd_` 前缀 | `sk_stdio.h` / `kstd_vector` |

### 📋 Git Commit 规范

```
<type>(<scope>): <subject>

type: feat|fix|docs|style|refactor|perf|test|build|revert
scope: 可选，影响的模块 (arch, device, libc)
subject: 不超过50字符，不加句号
```

### 📚 文档

- **工具链**: [docs/0_工具链.md](./docs/0_工具链.md)
- **系统启动**: [docs/1_系统启动.md](./docs/1_系统启动.md)
- **调试输出**: [docs/2_调试输出.md](./docs/2_调试输出.md)
- **中断**: [docs/3_中断.md](./docs/3_中断.md)
- **Dev Container**: [docs/docker.md](./docs/docker.md)
- **接口重构计划**: [docs/TODO_interface_refactor.md](./docs/TODO_interface_refactor.md)

## 🤝 贡献指南

我们欢迎所有形式的贡献！

### 🎯 贡献方式

| 方式 | 说明 |
|------|------|
| 🐛 **报告问题** | 通过 [GitHub Issues](https://github.com/Simple-XX/SimpleKernel/issues) 报告 Bug |
| 📐 **改进接口** | 提出更好的接口抽象和文档改进建议 |
| 🧪 **补充测试** | 为现有接口编写更完整的测试用例 |
| 📖 **完善文档** | 改进 Doxygen 注释、添加使用示例 |
| 🔧 **提交实现** | 提交接口的参考实现或替代实现 |

### 🔧 代码贡献流程

1. Fork 本仓库
2. 创建功能分支: `git checkout -b feat/amazing-feature`
3. 遵循代码规范进行开发
4. 确保所有测试通过
5. 提交变更: `git commit -m 'feat(scope): add amazing feature'`
6. 创建 Pull Request

## 📄 许可证

本项目采用多重许可证：

- **代码许可** - [MIT License](./LICENSE)
- **反 996 许可** - [Anti 996 License](https://github.com/996icu/996.ICU/blob/master/LICENSE)

---

<div align="center">

**⭐ 如果这个项目对您有帮助，请给我们一个 Star！**

**🤖 让 AI 帮你写内核，让你专注于理解操作系统原理！**

[🌟 Star 项目](https://github.com/Simple-XX/SimpleKernel) • [🐛 报告问题](https://github.com/Simple-XX/SimpleKernel/issues) • [💬 参与讨论](https://github.com/Simple-XX/SimpleKernel/discussions)

</div>
