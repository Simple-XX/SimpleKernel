[![codecov](https://codecov.io/gh/Simple-XX/SimpleKernel/graph/badge.svg?token=J7NKK3SBNJ)](https://codecov.io/gh/Simple-XX/SimpleKernel)
![workflow](https://github.com/Simple-XX/SimpleKernel/actions/workflows/workflow.yml/badge.svg)
![commit-activity](https://img.shields.io/github/commit-activity/t/Simple-XX/SimpleKernel)
![MIT License](https://img.shields.io/github/license/mashape/apistatus.svg)
[![LICENSE](https://img.shields.io/badge/license-Anti%20996-blue.svg)](https://github.com/996icu/996.ICU/blob/master/LICENSE)
[![996.icu](https://img.shields.io/badge/link-996.icu-red.svg)](https://996.icu)

[English](./README_ENG.md) | [中文](./README.md)

# SimpleKernel

**Interface-Driven OS Kernel for AI-Assisted Learning | Multi-Architecture: RISC-V 64, AArch64**

> 🤖 **Design Philosophy**: Define clear kernel interfaces, let AI generate the implementation — a new paradigm for learning operating systems

## 📖 Table of Contents

- [✨ Project Overview](#-project-overview)
- [🤖 AI-Oriented Design Philosophy](#-ai-oriented-design-philosophy)
- [🏛️ Interface Architecture Overview](#️-interface-architecture-overview)
- [🏗️ Supported Architectures](#️-supported-architectures)
- [🚀 Quick Start](#-quick-start)
- [📂 Project Structure](#-project-structure)
- [🎯 Learning Path](#-learning-path)
- [📦 Third-Party Dependencies](#-third-party-dependencies)
- [📝 Development Guide](#-development-guide)
- [🤝 Contributing](#-contributing)
- [📄 License](#-license)

## ✨ Project Overview

SimpleKernel is a **modern OS kernel project designed for AI-assisted learning**. Written in C++23, it supports RISC-V 64 and AArch64 architectures.

Unlike traditional OS teaching projects, SimpleKernel adopts an **Interface-Driven** design:

- **The project body is interface definitions** — complete header files (`.h/.hpp`) containing class declarations, pure virtual interfaces, type definitions, and Doxygen documentation
- **Implementation is done by AI** — you only need to understand the interface contracts, and let AI generate `.cpp` implementations from the interface docs
- **Reference implementations for comparison** — the project provides complete reference implementations to verify the correctness of AI-generated code

### 🌟 Core Highlights

| Feature | Description |
|---------|-------------|
| 🤖 **AI-First Design** | Interface docs serve as prompts — AI can generate complete implementations directly from header files |
| 📐 **Interface-Implementation Separation** | Headers contain only declarations and contracts; implementations live in separate `.cpp` files |
| 🌐 **Two-Architecture Support** | RISC-V 64, AArch64 — one set of interfaces adapting to different hardware |
| 🧪 **Test-Driven Verification** | GoogleTest test suites verify whether AI-generated implementations conform to interface contracts |
| 📖 **Complete Doxygen Documentation** | Every interface has responsibility descriptions, preconditions, postconditions, and usage examples |
| 🏗️ **Engineering Infrastructure** | CMake build, Dev Container environment, CI/CD, clang-format/clang-tidy |

## 🤖 AI-Oriented Design Philosophy

### Why "AI-Oriented"?

Traditional OS teaching projects follow: **read code → understand principles → mimic and modify**. This approach has several problems:

1. Kernel codebases are large — beginners easily get lost in implementation details
2. Modules are tightly coupled — difficult to understand individual subsystems independently
3. Implementing a module from scratch has a high barrier with long feedback cycles

SimpleKernel proposes a new paradigm: **read interface → understand contract → AI implements → test verifies**

```
┌─────────────────────────────────────────────────────────┐
│                 SimpleKernel Learning Flow                │
│                                                         │
│   ┌──────────┐    ┌──────────┐    ┌──────────┐         │
│   │ 📐 Inter- │───▶│ 🤖 AI    │───▶│ 🧪 Test  │         │
│   │ face Hdrs │    │ Generates│    │ Verifies │         │
│   │ + Doxygen │    │ Impl     │    │ Contract │         │
│   │           │    │ (.cpp)   │    │ GoogleTest│         │
│   └──────────┘    └──────────┘    └──────────┘         │
│        │                               │                │
│        │         ┌──────────┐          │                │
│        └────────▶│ 📚 Ref   │◀─────────┘                │
│                  │ Impl     │                           │
│                  └──────────┘                           │
└─────────────────────────────────────────────────────────┘
```

### Core Workflow

#### 1️⃣ Read Interface, Understand Contract

Each module's header file contains complete interface documentation:

```cpp
/**
 * @brief Interrupt subsystem abstract base class
 *
 * All architecture interrupt handlers must implement this interface.
 *
 * @pre  Hardware interrupt controller has been initialized
 * @post Can register interrupt handlers via RegisterInterruptFunc
 *
 * Known implementations: PLIC (RISC-V), GIC (AArch64)
 */
class InterruptBase {
public:
  virtual ~InterruptBase() = default;

  /// Execute interrupt handling
  virtual void Do(uint64_t cause, cpu_io::TrapContext* context) = 0;

  /// Register interrupt handler function
  virtual void RegisterInterruptFunc(uint64_t cause, InterruptFunc func) = 0;
};
```

#### 2️⃣ Let AI Implement

Provide the header file as context to an AI (e.g., GitHub Copilot, ChatGPT, Claude) and ask it to generate the `.cpp` implementation. The Doxygen comments in the interface are the best prompt.

#### 3️⃣ Test and Verify

Run the project's built-in test suite to verify the AI-generated implementation conforms to the interface contract:

```shell
cmake --preset build_riscv64
cd build_riscv64 && make unit-test
```

#### 4️⃣ Compare with Reference Implementation

If tests fail, refer to the project's reference implementation for comparison and learning.

### Integration with AI Tools

| Scenario | Usage |
|----------|-------|
| **GitHub Copilot** | Open the header file, let Copilot auto-complete the implementation in the corresponding `.cpp` |
| **ChatGPT / Claude** | Paste header file contents as context, request a complete `.cpp` implementation |
| **Copilot Chat / Cursor** | Select the interface in the IDE, ask AI to explain contract meaning or generate implementation |
| **Self-Study** | Think about the implementation first, then let AI generate it, and compare differences |

## 🏛️ Interface Architecture Overview

SimpleKernel's interfaces are organized into the following layers:

```
┌──────────────────────────────────────────┐
│          Application / Syscall Layer      │
│         syscall.h · SyscallInit          │
├──────────────────────────────────────────┤
│            Task Management Layer          │
│  TaskManager · SchedulerBase · Mutex     │
│  CfsScheduler · FifoScheduler · RR ...   │
├──────────────────────────────────────────┤
│          Memory Management Layer          │
│  VirtualMemory · PhysicalMemory          │
│  MapPage · UnmapPage · AllocFrame        │
├──────────────────────────────────────────┤
│          Interrupt / Exception Layer      │
│  InterruptBase · RegisterInterruptFunc   │
│  TimerInit · InterruptInit               │
├──────────────────────────────────────────┤
│               Device Framework Layer           │
│  DeviceManager · DriverRegistry               │
│  PlatformBus · Ns16550aDriver · VirtioBlk     │
├──────────────────────────────────────────┤
│       Architecture Abstraction (arch.h)   │
│  ArchInit · InterruptInit · TimerInit    │
│  EarlyConsole (auto-set during global    │
│               construction phase)         │
├──────────────────────────────────────────┤
│         Runtime Support Libraries         │
│  libc (sk_stdio.h, sk_string.h, ...)     │
│  libcxx (kstd_vector, __cxa_*, ...)      │
├──────────────────────────────────────────┤
│            Hardware / QEMU                │
│  RISC-V 64 · AArch64                    │
└──────────────────────────────────────────┘
```

### Key Interface Files

| Interface File | Responsibility | Implementation File |
|---------------|---------------|-------------------|
| `src/arch/arch.h` | Architecture-independent unified entry | Each `src/arch/{arch}/` directory |
| `src/include/interrupt_base.h` | Interrupt subsystem abstract base class | `src/arch/{arch}/interrupt.cpp` |
| `src/device/include/device_manager.hpp` | Device manager | header-only |
| `src/device/include/driver_registry.hpp` | Driver registry | header-only |
| `src/device/include/platform_bus.hpp` | Platform bus (FDT enumeration) | header-only |
| `src/device/include/driver/ns16550a_driver.hpp` | NS16550A UART driver | header-only (Probe/Remove pattern) |
| `src/include/virtual_memory.hpp` | Virtual memory management interface | `src/virtual_memory.cpp` |
| `src/include/kernel_fdt.hpp` | Device tree parsing interface | `src/kernel_fdt.cpp` |
| `src/include/kernel_elf.hpp` | ELF parsing interface | `src/kernel_elf.cpp` |
| `src/task/include/scheduler_base.hpp` | Scheduler abstract base class | `cfs_scheduler.cpp` etc. |
| `src/include/spinlock.hpp` | Spinlock interface | header-only (performance) |
| `src/include/mutex.hpp` | Mutex interface | `src/task/mutex.cpp` |

> 📋 See [docs/TODO_interface_refactor.md](./docs/TODO_interface_refactor.md) for the complete interface refactoring plan.

## 🏗️ Supported Architectures

| Architecture | Boot Chain | Serial | Interrupt Controller | Timer |
|:---:|:---:|:---:|:---:|:---:|
| **RISC-V 64** | U-Boot + OpenSBI | SBI Call | Direct Mode | SBI Timer |
| **AArch64** | U-Boot + ATF + OP-TEE | PL011 | GICv3 | Generic Timer |

## 🚀 Quick Start

### 📋 System Requirements

- **Operating System**: Linux (Ubuntu 24.04 recommended) or macOS
- **Container Engine**: Docker or compatible container runtime
- **Toolchain**: Included in Dev Container (GCC 14 cross-compilers, CMake, QEMU, etc.)
- **AI Tools (recommended)**: GitHub Copilot / ChatGPT / Claude

### 🛠️ Environment Setup

**Option 1: Using Dev Container (Recommended)**

```shell
# 1. Clone the project
git clone https://github.com/simple-xx/SimpleKernel.git
cd SimpleKernel

# 2. Open in VS Code and reopen in container
#    Install Dev Containers extension, click the >< icon at bottom-left
#    Select "Reopen in Container"

# Or use CLI
npm install -g @devcontainers/cli
devcontainer up --workspace-folder .
devcontainer exec --workspace-folder . bash
```

> Also supports **GitHub Codespaces**: Click Code → Codespaces → Create codespace on main
>
> See [Dev Container documentation](./docs/docker.md) for details.

**Option 2: Local Environment**

Refer to [Toolchain Documentation](./docs/0_工具链.md) for local development environment setup.

### ⚡ Build and Run

```shell
cd SimpleKernel

# Select target architecture (RISC-V 64 example)
cmake --preset build_riscv64
cd build_riscv64

# Build kernel
make SimpleKernel

# Run in QEMU emulator
make run

# Run unit tests (verify your implementation)
make unit-test
```

**Supported Architecture Presets:**
- `build_riscv64` - RISC-V 64-bit architecture
- `build_aarch64` - ARM 64-bit architecture

### 🎯 AI-Assisted Development Workflow

```shell
# 1. Open project in VS Code (GitHub Copilot extension recommended)
code ./SimpleKernel

# 2. Read interface definitions in header files (e.g., src/include/virtual_memory.hpp)

# 3. Create/edit the corresponding .cpp file, let AI generate implementation from the interface

# 4. Build and verify
cd build_riscv64 && make SimpleKernel

# 5. Run tests
make unit-test

# 6. Run in QEMU, observe behavior
make run
```

## 📂 Project Structure

```
SimpleKernel/
├── src/                        # Kernel source code
│   ├── include/                # 📐 Public interface headers (project core)
│   │   ├── virtual_memory.hpp  #   Virtual memory management interface
│   │   ├── kernel_fdt.hpp      #   Device tree parsing interface
│   │   ├── kernel_elf.hpp      #   ELF parsing interface
│   │   ├── spinlock.hpp        #   Spinlock interface
│   │   ├── mutex.hpp           #   Mutex interface
│   │   └── ...
│   ├── arch/                   # Architecture-specific code
│   │   ├── arch.h              # 📐 Architecture-independent unified interface
│   │   ├── aarch64/            #   AArch64 implementation
│   │   └── riscv64/            #   RISC-V 64 implementation
│   ├── device/                 # Device management framework
│   │   ├── include/            # 📐 Device framework interfaces (DeviceManager, DriverRegistry, Bus, etc.)
│   │   │   └── driver/         #   Concrete drivers (ns16550a_driver.hpp, virtio_blk_driver.hpp)
│   │   └── device.cpp          #   Device initialization entry (DeviceInit)
│   ├── task/                   # Task management
│   │   ├── include/            # 📐 Scheduler interfaces (SchedulerBase, etc.)
│   │   └── ...                 #   Scheduler implementations
│   ├── libc/                   # Kernel C standard library
│   └── libcxx/                 # Kernel C++ runtime
├── tests/                      # 🧪 Test suite
│   ├── unit_test/              #   Unit tests
│   ├── integration_test/       #   Integration tests
│   └── system_test/            #   System tests (QEMU-based)
├── docs/                        # 📚 Documentation
│   ├── TODO_interface_refactor.md  # Interface refactoring plan
│   └── ...
├── cmake/                      # CMake build configuration
├── 3rd/                        # Third-party dependencies (Git Submodule)
└── tools/                      # Build tools and templates
```

> Directories/files marked with 📐 are **interface definitions** — these are what you should focus on reading.

## 🎯 Learning Path

We recommend learning and implementing modules in the following order:

### Phase 1: Infrastructure (Boot)

| Module | Interface File | Difficulty | Description |
|--------|---------------|:---:|-------------|
| Early Console | `src/arch/arch.h` comments | ⭐ | Earliest output, understand global construction |
| Serial Driver | `ns16550a_driver.hpp` | ⭐⭐ | Implement Probe/Remove, understand device framework and MMIO |
| Device Tree Parsing | `kernel_fdt.hpp` | ⭐⭐ | Parse hardware info, understand FDT format |
| ELF Parsing | `kernel_elf.hpp` | ⭐⭐ | Symbol table parsing, used for stack backtrace |

### Phase 2: Interrupt System

| Module | Interface File | Difficulty | Description |
|--------|---------------|:---:|-------------|
| Interrupt Base | `interrupt_base.h` | ⭐⭐ | Understand unified interrupt abstraction |
| Interrupt Controller | Per-arch driver headers | ⭐⭐⭐ | GIC/PLIC hardware programming |
| Timer Interrupt | `arch.h → TimerInit` | ⭐⭐ | Timer configuration, tick-driven |

### Phase 3: Memory Management

| Module | Interface File | Difficulty | Description |
|--------|---------------|:---:|-------------|
| Virtual Memory | `virtual_memory.hpp` | ⭐⭐⭐ | Page table management, address mapping |
| Physical Memory | Related interfaces | ⭐⭐⭐ | Frame allocator, buddy system |

### Phase 4: Task Management (Thread/Task)

| Module | Interface File | Difficulty | Description |
|--------|---------------|:---:|-------------|
| Spinlock | `spinlock.hpp` | ⭐⭐ | Atomic operations, multi-core synchronization |
| Mutex | `mutex.hpp` | ⭐⭐⭐ | Task-blocking based lock |
| Scheduler | `scheduler_base.hpp` | ⭐⭐⭐ | CFS/FIFO/RR scheduling algorithms |

### Phase 5: System Calls

| Module | Interface File | Difficulty | Description |
|--------|---------------|:---:|-------------|
| System Calls | `arch.h → SyscallInit` | ⭐⭐⭐ | User/kernel mode switching |

## 📦 Third-Party Dependencies

| Dependency | Purpose |
|-----------|---------|
| [google/googletest](https://github.com/google/googletest.git) | Testing framework |
| [charlesnicholson/nanoprintf](https://github.com/charlesnicholson/nanoprintf.git) | printf implementation |
| [MRNIU/cpu_io](https://github.com/MRNIU/cpu_io.git) | CPU I/O operations |
| [riscv-software-src/opensbi](https://github.com/riscv-software-src/opensbi.git) | RISC-V SBI implementation |
| [MRNIU/opensbi_interface](https://github.com/MRNIU/opensbi_interface.git) | OpenSBI interface |
| [u-boot/u-boot](https://github.com/u-boot/u-boot.git) | Universal bootloader |
| [OP-TEE/optee_os](https://github.com/OP-TEE/optee_os.git) | OP-TEE operating system |
| [ARM-software/arm-trusted-firmware](https://github.com/ARM-software/arm-trusted-firmware.git) | ARM Trusted Firmware |
| [dtc/dtc](https://git.kernel.org/pub/scm/utils/dtc/dtc.git) | Device Tree Compiler |
| [MRNIU/bmalloc](https://github.com/MRNIU/bmalloc.git) | Memory allocator |
| [MRNIU/MPMCQueue](https://github.com/MRNIU/MPMCQueue.git) | Lock-free MPMC queue |
| [MRNIU/device_framework](https://github.com/MRNIU/device_framework.git) | Device management framework |

## 📝 Development Guide

### 🎨 Code Style

- **Language Standard**: C23 / C++23
- **Coding Standard**: [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- **Auto Formatting**: `.clang-format` + `.clang-tidy`
- **Comment Standard**: Doxygen style; interface files must contain complete contract documentation

### Naming Conventions

| Type | Style | Example |
|------|-------|---------|
| Files | lower_snake_case | `kernel_log.hpp` |
| Classes/Structs | PascalCase | `TaskManager` |
| Functions | PascalCase / snake_case | `ArchInit` / `sys_yield` |
| Variables | snake_case | `per_cpu_data` |
| Macros | SCREAMING_SNAKE | `SIMPLEKERNEL_DEBUG` |
| Constants | kCamelCase | `kPageSize` |
| Kernel libc/libc++ headers | libc: `sk_` prefix, libcxx: `kstd_` prefix | `sk_stdio.h` / `kstd_vector` |

### 📋 Git Commit Convention

```
<type>(<scope>): <subject>

type: feat|fix|docs|style|refactor|perf|test|build|revert
scope: optional, affected module (arch, device, libc)
subject: max 50 chars, no period
```

### 📚 Documentation

- **Toolchain**: [docs/0_工具链.md](./docs/0_工具链.md)
- **System Boot**: [docs/1_系统启动.md](./docs/1_系统启动.md)
- **Debug Output**: [docs/2_调试输出.md](./docs/2_调试输出.md)
- **Interrupts**: [docs/3_中断.md](./docs/3_中断.md)
- **Dev Container**: [docs/docker.md](./docs/docker.md)
- **Interface Refactoring Plan**: [docs/TODO_interface_refactor.md](./docs/TODO_interface_refactor.md)

## 🤝 Contributing

We welcome all forms of contributions!

### 🎯 Ways to Contribute

| Method | Description |
|--------|-------------|
| 🐛 **Report Issues** | Report bugs via [GitHub Issues](https://github.com/Simple-XX/SimpleKernel/issues) |
| 📐 **Improve Interfaces** | Suggest better interface abstractions and documentation improvements |
| 🧪 **Add Tests** | Write more comprehensive test cases for existing interfaces |
| 📖 **Improve Documentation** | Enhance Doxygen comments, add usage examples |
| 🔧 **Submit Implementations** | Submit reference or alternative implementations of interfaces |

### 🔧 Code Contribution Workflow

1. Fork this repository
2. Create a feature branch: `git checkout -b feat/amazing-feature`
3. Follow coding standards during development
4. Ensure all tests pass
5. Commit changes: `git commit -m 'feat(scope): add amazing feature'`
6. Create a Pull Request

## 📄 License

This project is dual-licensed:

- **Code License** - [MIT License](./LICENSE)
- **Anti-996 License** - [Anti 996 License](https://github.com/996icu/996.ICU/blob/master/LICENSE)

---

<div align="center">

**⭐ If this project helps you, please give us a Star!**

**🤖 Let AI write the kernel, so you can focus on understanding OS principles!**

[🌟 Star the Project](https://github.com/Simple-XX/SimpleKernel) • [🐛 Report Issues](https://github.com/Simple-XX/SimpleKernel/issues) • [💬 Join Discussions](https://github.com/Simple-XX/SimpleKernel/discussions)

</div>
