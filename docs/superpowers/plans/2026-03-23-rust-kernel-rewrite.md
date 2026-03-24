# SimpleKernel Rust 重写 — 执行计划

> **给 AI 工作者：** 推荐使用 `superpowers:subagent-driven-development` 或 `superpowers:executing-plans` 技能逐步实施。步骤使用 `- [ ]` 语法以便跟踪进度。

**设计文档：** 代码盘点、模式映射、依赖替换、项目结构、构建系统、测试策略、风险评估、代码规范、工程化补充等全部设计细节见 → [`docs/rust-rewrite/00-概述.md`](../../rust-rewrite/00-概述.md)

**阶段文档：** 每个阶段的详细步骤、退出标准、QEMU 预期输出、验证命令见 → `docs/rust-rewrite/P0-P7` 各文档

---

## 0. 关键决策

| 日期 | 决策 | 影响 |
|------|------|------|
| **2026-03-24** | **完全迁移到 Rust，抛弃 C++ 版本** | 在独立迁移分支 `feat/rust` 上开发。C++ 源码（`src/`）在重写期间保留作为实现参考，不主动删除。全部阶段完成并验证后合入 main，届时统一清理 C++ 代码、CMake 构建系统及 C++ 第三方子模块。详见 [00-概述.md §10.1](../../rust-rewrite/00-概述.md#101-迁移策略)。 |

---

## 1. 阶段概览

| 阶段 | 持续时间 | 复杂度 | 可并行性 | 详细文档 |
|-------|----------|--------|----------|----------|
| P0: 项目骨架与构建系统 | 1 周 | 低 | 否（基础） | [P0](../../rust-rewrite/P0-项目骨架与构建系统.md) |
| P1: 启动与早期控制台 | 2 周 | 中 | 否（依赖 P0） | [P1](../../rust-rewrite/P1-启动与早期控制台.md) |
| P2: 核心基础设施 | 1 周 | 中低 | 否（依赖 P1） | [P2](../../rust-rewrite/P2-核心基础设施.md) |
| P3: 内存管理 | 2 周 | 高 | 否（依赖 P2） | [P3](../../rust-rewrite/P3-内存管理.md) |
| P4: 中断与定时器 | 2 周 | 高 | 否（依赖 P3） | [P4](../../rust-rewrite/P4-中断与定时器.md) |
| P5: 任务管理与调度 | 3 周 | 极高 | 否（依赖 P4） | [P5](../../rust-rewrite/P5-任务管理与调度.md) |
| P6: 设备框架与驱动 | 3 周 | 中高 | 是（与 P5/P7 并行） | [P6](../../rust-rewrite/P6-设备框架与驱动.md) |
| P7: 文件系统 | 2 周 | 中 | 是（与 P6 并行） | [P7](../../rust-rewrite/P7-文件系统.md) |
| **总计** | **~16 周** | | | |

## 2. 依赖图

```
P0 ──→ P1 ──→ P2 ──→ P3 ──→ P4 ──┬──→ P5
                                    │
                                    ├──→ P6 (设备框架核心可在 P4 后开始)
                                    │         │
                                    │         └→ VirtIO 阻塞 I/O 需等 P5
                                    │
                                    └──→ P7 (VFS+RamFS 可在 P4 后开始)
                                              │
                                              └→ FatFS 需等 P6 VirtIO 完成
```

## 3. 可运行里程碑原则

每个阶段结束时，内核必须能在 QEMU 中运行并产生可观察的输出。

| 阶段 | 运行后你会看到什么 |
|-------|-------------------|
| P0 | 内核启动后停在死循环（QEMU 无输出，不崩溃） |
| P1 | 串口打印 "Hello SimpleKernel"，显示 FDT 解析结果 |
| P2 | 同 P1，但 panic 时显示调用栈，SpinLock 可用 |
| P3 | 同 P2，内核启用分页后继续运行，堆分配可用 |
| P4 | 定时器中断触发日志输出（每秒 ~1000 次 tick） |
| P5 | 多个内核线程在运行，调度器选择任务，idle 线程让出 CPU |
| P6 | 设备从 FDT 枚举，UART 通过驱动框架输出，VirtIO 块设备读写 |
| P7 | 文件系统挂载，可以创建/读取/删除文件 |

**每个阶段的验证流程：**
```bash
cargo build --target targets/riscv64gc-simplekernel.json  # 编译
cargo xtask run --arch riscv64                             # 运行
# 观察串口输出，对照该阶段 P* 文档的 "QEMU 预期输出"
# Ctrl+A, X 退出 QEMU
```

## 4. 需要尽早做的决策

| 决策 | 选项 | 建议 |
|------|------|------|
| 堆分配器 | `linked_list_allocator` vs `buddy_system_allocator` | `buddy_system_allocator` — 页面粒度分配效果更好 |
| 集合类型 | `heapless` (固定容量) vs `alloc` (堆) | 混合：热路径用 `heapless`，灵活结构用 `alloc` |
| 中断处理 | trait 对象 vs 枚举 | 已知集合用枚举；可扩展性需求用 trait 对象 |
| 单例模式 | `spin::Once<T>` vs `static mut` + `unsafe` | `spin::Once<T>` — 安全且地道 |
| 错误处理 | 单一 `KernelError` vs 按模块定义错误 | 单一 `KernelError` + `ErrorCode` 枚举 — 匹配 C++ 设计 |
| CPU 架构 crate | `riscv`/`aarch64-cpu` vs 自定义内联汇编 | 尽可能用 crate；特殊需求用自定义汇编 |

## 5. 风险评估

> 完整版见 [00-概述.md §7](../../rust-rewrite/00-概述.md#7-风险评估与缓解)

### 高风险

| 风险 | 缓解措施 |
|------|----------|
| 汇编 ↔ Rust ABI 不匹配 | `#[repr(C)]` + `offset_of!` 编译时断言验证结构体布局 |
| 固件启动链不匹配 | P0 即用完整 U-Boot FIT 链测试，不用 `-bios none` |
| SMP 启动切换 | P4 前用 2+ 核测试 SBI `hart_start` / PSCI |
| Rust 所有权 vs 调度器结构 | P2 强制 TCB 所有权原型准入门槛 |
| 宿主机 vs 目标机测试差异 | 每阶段 QEMU 冒烟测试，不仅靠 `cargo test` |

### 中等风险

| 风险 | 缓解措施 |
|------|----------|
| Nightly Rust 不稳定性 | `rust-toolchain.toml` 固定版本 |
| 全局分配器初始化顺序 | 早期启动用静态分配，延迟激活 `#[global_allocator]` |
| Rust 二进制体积 | LTO + `panic = "abort"` + `codegen-units = 1` |

## 6. 命令参考

| 旧命令 (CMake) | 新命令 (Cargo) |
|------------|-------------|
| `cmake --preset build_riscv64` | `cargo build --target targets/riscv64gc-simplekernel.json` |
| `cmake --preset build_aarch64` | `cargo build --target targets/aarch64-simplekernel.json` |
| `make SimpleKernel` | `cargo build --release` |
| `make run` | `cargo xtask run --arch riscv64` |
| `make debug` | `cargo xtask debug --arch riscv64` |
| `make unit-test` | `cargo test` |
| `make coverage` | `cargo llvm-cov` |
| `pre-commit run --all-files` | `cargo fmt --check && cargo clippy` |

## 7. Nightly 特性

```rust
#![feature(naked_functions)]         // 中断向量入口 (tracking #32408)
#![feature(alloc_error_handler)]     // OOM 处理 (tracking #51941)
#![feature(custom_test_frameworks)]  // no_std 测试 (tracking #50297)
#![feature(asm_const)]               // 内联汇编常量 (tracking #93332)
```

已稳定（无需 `#![feature(...)]`）：
- `asm!` — Rust 1.59+
- `panic_info_message` — Rust 1.94.0+
