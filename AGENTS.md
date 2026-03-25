# AGENTS.md — SimpleKernel

## OVERVIEW
Interface-driven OS kernel for AI-assisted learning. Rust (`no_std`, `no_main`), freestanding, nightly toolchain. Two architectures: riscv64, aarch64. Traits define contracts (doc comments with `# Safety`/`# Errors`/`# Panics`), AI generates `impl` blocks, tests verify compliance.

> **迁移状态**：项目正在从 C++ 完全迁移到 Rust。C++ 源码（`src/`）保留作为实现参考，但不再维护。所有新开发均在 Rust（`src/`）中进行。详见 `docs/rust-rewrite/00-概述.md`。

## STRUCTURE
```
src/                  # Kernel source — trait definitions + implementations
src/arch/             # Per-architecture code (riscv64/, aarch64/)
xtask/                # Build tool (cargo xtask run/build/debug/firmware)
tests/                # System tests (QEMU)
docs/rust-rewrite/    # Design docs, phase plans (P0-P7)
3rd/                  # Git submodules (opensbi, u-boot, optee, atf, dtc — firmware only)
```

## WHERE TO LOOK
- **Implementing a module** → Read trait definition in the module's `mod.rs` or dedicated trait file first
- **Adding a driver** → `src/device/` for examples, `Driver` trait for registration pattern
- **Adding a scheduler** → `src/task/scheduler/mod.rs` for `Scheduler` trait
- **Boot flow** → `src/main.rs`: `_start` → `arch::bootstrap` → `arch_init` → `MemoryInit` → `InterruptInit` → `DeviceInit` → `FileSystemInit` → `Schedule()`
- **Error handling** → `KResult<T> = Result<T, ErrorCode>` in `src/error.rs`
- **Logging** → `log::info!()` / `log::debug!()` via `log` crate, backend in `src/logging.rs`
- **C++ reference** → `src/` directory (read-only, for understanding original design intent)
- **Design overview** → `docs/rust-rewrite/00-概述.md` (master plan with all design decisions)
- **Phase details** → `docs/rust-rewrite/P0-P7` (step-by-step implementation plans)

## CODE MAP
| Module | Purpose | Key Files |
|--------|---------|-----------|
| `src/main.rs` | `#![no_std]` `#![no_main]` entry, `_start` | main entry point |
| `src/arch/` | Arch-agnostic dispatch via `cfg` | `mod.rs` + `{riscv64,aarch64}/` |
| `src/arch/{arch}/init.rs` | ArchInit, ArchInitSMP | per-arch boot sequence |
| `src/arch/{arch}/console.rs" | Early console (SBI / PL011) | UART output |
| `src/arch/{arch}/interrupt.rs` | PLIC/GIC + trap dispatch | interrupt handling |
| `src/arch/{arch}/timer.rs` | Timer init + tick handler | timer subsystem |
| `src/arch/{arch}/context.rs` | TrapContext, InitTaskContext | `#[repr(C)]` structs |
| `src/arch/{arch}/syscall.rs` | ecall/svc handling | syscall dispatch |
| `src/arch/{arch}/backtrace.rs` | Stack unwinding | debug support |
| `src/memory/` | Virtual/physical memory, heap | page tables, frame allocator, `#[global_allocator]` |
| `src/task/` | TaskManager, TCB, schedulers | CFS/FIFO/RR, clone/exit/wait/sleep/signal |
| `src/task/scheduler/` | `Scheduler` trait + implementations | scheduling algorithms |
| `src/device/` | DeviceManager, DriverRegistry, drivers | device framework |
| `src/device/virtio/` | VirtIO subsystem (MMIO, queues, blk) | block device I/O |
| `src/fs/` | VFS, RamFS, FatFS | filesystem layer |
| `src/sync/spinlock.rs` | SpinLock (interrupt-aware, lock levels) | custom implementation |
| `src/error.rs` | `ErrorCode`, `KResult<T>` | error handling |
| `src/logging.rs` | `log` crate backend + ANSI colors | kernel logging |
| `src/config.rs` | Kernel constants (`MAX_CORE_COUNT`, etc.) | configuration |
| `src/per_cpu.rs` | Per-CPU data + BasicInfo | SMP support |
| `src/fdt.rs` | Device tree parser (`fdt` crate wrapper) | hardware discovery |
| `src/elf.rs` | ELF symbol table parser | backtrace support |
| `src/panic.rs` | Panic handler + observer pattern | error recovery |
| `src/lang_items.rs` | `#[panic_handler]` | Rust runtime |

## CONVENTIONS

> Full Rust coding conventions: `docs/rust-rewrite/00-概述.md` §9

- **Language**: Rust nightly, `#![no_std]`, `#![no_main]`, edition 2024
- **Naming**: `snake_case` functions/methods, `PascalCase` types/traits/enums, `SCREAMING_SNAKE_CASE` constants
- **Formatting**: `rustfmt.toml` (100 char width), enforce via `cargo fmt`
- **Linting**: `cargo clippy -- -D warnings`
- **Doc comments**: `///` with `# Safety`, `# Errors`, `# Panics` sections for public APIs（节标题保留英文，内容用中文）
- **注释语言**: 所有注释和文档注释使用中文；`// SAFETY:` 前缀保留英文（Rust 社区惯例），其后说明用中文
- **Error handling**: `Result<T, ErrorCode>` + `?` operator; `.expect("reason")` not `.unwrap()`
- **Unsafe**: Every `unsafe` block MUST have `// SAFETY:` comment explaining invariants; minimize scope
- **Singletons**: `spin::Once<T>` with `call_once()` / `get()`
- **Sync**: Custom `SpinLock<T>` (interrupt-aware), NOT `spin::Mutex` for kernel mutual exclusion
- **Assembly**: `.S` files compiled via `cc` crate in `build.rs`; `#[repr(C)]` for ABI-compatible structs
- **Attributes**: Rust 2024 edition syntax — `#[unsafe(no_mangle)]` (not `#[no_mangle]`)

## ANTI-PATTERNS

- **NO** `.unwrap()` — use `.expect("reason")` or `?`
- **NO** `unsafe` without `// SAFETY:` comment
- **NO** modifying trait definitions to embed implementation (traits = contracts)
- **NO** `spin::Mutex` for kernel mutual exclusion (doesn't disable interrupts)
- **NO** `static mut` — use `SyncUnsafeCell` or `spin::Once<T>`
- **NO** empty `unsafe {}` blocks to bypass borrow checker
- **NO** suppressing warnings with `#[allow(...)]` without justification
- **NO** modifying `src/` (legacy C++ code, read-only reference)

## UNIQUE STYLES
- `spin::Once<T>` with named statics: `TASK_MANAGER.call_once(|| ...)`, `TASK_MANAGER.get().unwrap()`
- `SpinLockGuard<'_, T>` RAII locking (disables/restores interrupts on acquire/release)
- `KResult<T> = Result<T, ErrorCode>` project-wide type alias
- Per-architecture code selected via `#[cfg(target_arch = "...")]`
- Cargo workspace: root package (kernel) + `xtask` (build tool)

## COMMANDS
```bash
# Build kernel
cargo xtask build --arch riscv64
cargo xtask build --arch aarch64

# Run in QEMU (via xtask — handles FIT image + TFTP + QEMU)
cargo xtask run --arch riscv64
cargo xtask run --arch aarch64

# Debug (GDB on localhost:1234)
cargo xtask debug --arch riscv64

# Unit tests (x86_64 host only)
cargo test

# Format + lint check
cargo fmt --check && cargo clippy -- -D warnings

# Documentation
cargo doc --no-deps
```

## NOTES
- Interface-driven: traits are contracts, `impl` blocks are implementations AI generates
- Boot chains differ: riscv64 (U-Boot SPL→OpenSBI→U-Boot), aarch64 (U-Boot→ATF→OP-TEE)
- aarch64 needs two serial terminal tasks (::54320, ::54321) before `cargo xtask run --arch aarch64`
- Unit tests run on x86_64 host only (`cargo test`) — system tests use QEMU (`cargo xtask run`)
- Git commits: `<type>(<scope>): <subject>` with `--signoff`
- Debug: use `cargo xtask debug` + GDB, QEMU logs in build output
- Design docs: `docs/rust-rewrite/00-概述.md` is the master reference for all design decisions
- Phase plans: `docs/rust-rewrite/P0-P7` for step-by-step implementation guides
- C++ code in `src/` is frozen — reference only, will be removed when migration completes
