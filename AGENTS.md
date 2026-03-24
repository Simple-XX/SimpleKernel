# AGENTS.md — SimpleKernel

## OVERVIEW
Interface-driven OS kernel for AI-assisted learning. Rust (`no_std`, `no_main`), freestanding, nightly toolchain. Two architectures: riscv64, aarch64. Traits define contracts (doc comments with `# Safety`/`# Errors`/`# Panics`), AI generates `impl` blocks, tests verify compliance.

> **迁移状态**：项目正在从 C++ 完全迁移到 Rust。C++ 源码（`src/`）保留作为实现参考，但不再维护。所有新开发均在 Rust（`kernel/`）中进行。详见 `docs/rust-rewrite/00-概述.md`。

## STRUCTURE
```
kernel/src/           # Main kernel crate — trait definitions + implementations
kernel/src/arch/      # Per-architecture code (riscv64/, aarch64/)
kernel/src/device/    # Device framework, drivers
kernel/src/task/      # Schedulers, TCB, sync
kernel/src/fs/        # VFS, RamFS, FatFS
kernel/src/memory/    # Virtual/physical memory management
kernel/src/sync/      # SpinLock, Mutex
xtask/                # Build tool (cargo xtask run/build/debug)
targets/              # Custom target JSON specs (riscv64, aarch64)
tests/                # System tests (QEMU)
docs/rust-rewrite/    # Design docs, phase plans (P0-P7)
3rd/                  # Git submodules (opensbi, u-boot, optee, atf, dtc — firmware only)
src/                  # [LEGACY] C++ source — read-only reference, do NOT modify
```

## WHERE TO LOOK
- **Implementing a module** → Read trait definition in the module's `mod.rs` or dedicated trait file first
- **Adding a driver** → `kernel/src/device/` for examples, `Driver` trait for registration pattern
- **Adding a scheduler** → `kernel/src/task/scheduler/mod.rs` for `Scheduler` trait
- **Boot flow** → `kernel/src/main.rs`: `_start` → `arch::bootstrap` → `arch_init` → `MemoryInit` → `InterruptInit` → `DeviceInit` → `FileSystemInit` → `Schedule()`
- **Error handling** → `KResult<T> = Result<T, ErrorCode>` in `kernel/src/error.rs`
- **Logging** → `log::info!()` / `log::debug!()` via `log` crate, backend in `kernel/src/logging.rs`
- **C++ reference** → `src/` directory (read-only, for understanding original design intent)
- **Design overview** → `docs/rust-rewrite/00-概述.md` (master plan with all design decisions)
- **Phase details** → `docs/rust-rewrite/P0-P7` (step-by-step implementation plans)

## CODE MAP
| Module | Purpose | Key Files |
|--------|---------|-----------|
| `kernel/src/main.rs` | `#![no_std]` `#![no_main]` entry, `_start` | main entry point |
| `kernel/src/arch/` | Arch-agnostic dispatch via `cfg` | `mod.rs` + `{riscv64,aarch64}/` |
| `kernel/src/arch/{arch}/init.rs` | ArchInit, ArchInitSMP | per-arch boot sequence |
| `kernel/src/arch/{arch}/console.rs` | Early console (SBI / PL011) | UART output |
| `kernel/src/arch/{arch}/interrupt.rs` | PLIC/GIC + trap dispatch | interrupt handling |
| `kernel/src/arch/{arch}/timer.rs` | Timer init + tick handler | timer subsystem |
| `kernel/src/arch/{arch}/context.rs` | TrapContext, InitTaskContext | `#[repr(C)]` structs |
| `kernel/src/arch/{arch}/syscall.rs` | ecall/svc handling | syscall dispatch |
| `kernel/src/arch/{arch}/backtrace.rs` | Stack unwinding | debug support |
| `kernel/src/memory/` | Virtual/physical memory, heap | page tables, frame allocator, `#[global_allocator]` |
| `kernel/src/task/` | TaskManager, TCB, schedulers | CFS/FIFO/RR, clone/exit/wait/sleep/signal |
| `kernel/src/task/scheduler/` | `Scheduler` trait + implementations | scheduling algorithms |
| `kernel/src/device/` | DeviceManager, DriverRegistry, drivers | device framework |
| `kernel/src/device/virtio/` | VirtIO subsystem (MMIO, queues, blk) | block device I/O |
| `kernel/src/fs/` | VFS, RamFS, FatFS | filesystem layer |
| `kernel/src/sync/spinlock.rs` | SpinLock (interrupt-aware, lock levels) | custom implementation |
| `kernel/src/error.rs` | `ErrorCode`, `KResult<T>` | error handling |
| `kernel/src/logging.rs` | `log` crate backend + ANSI colors | kernel logging |
| `kernel/src/config.rs` | Kernel constants (`MAX_CORE_COUNT`, etc.) | configuration |
| `kernel/src/per_cpu.rs` | Per-CPU data + BasicInfo | SMP support |
| `kernel/src/fdt.rs` | Device tree parser (`fdt` crate wrapper) | hardware discovery |
| `kernel/src/elf.rs` | ELF symbol table parser | backtrace support |
| `kernel/src/panic.rs` | Panic handler + observer pattern | error recovery |
| `kernel/src/lang_items.rs` | `#[panic_handler]` | Rust runtime |

## CONVENTIONS

> Full Rust coding conventions: `docs/rust-rewrite/00-概述.md` §9

- **Language**: Rust nightly, `#![no_std]`, `#![no_main]`, edition 2024
- **Naming**: `snake_case` functions/methods, `PascalCase` types/traits/enums, `SCREAMING_SNAKE_CASE` constants
- **Formatting**: `rustfmt.toml` (100 char width), enforce via `cargo fmt`
- **Linting**: `cargo clippy -- -D warnings`
- **Doc comments**: `///` with `# Safety`, `# Errors`, `# Panics` sections for public APIs
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
- Cargo workspace: `kernel` (main crate) + `xtask` (build tool)

## COMMANDS
```bash
# Build kernel
cargo build --target targets/riscv64gc-simplekernel.json
cargo build --target targets/aarch64-simplekernel.json

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
