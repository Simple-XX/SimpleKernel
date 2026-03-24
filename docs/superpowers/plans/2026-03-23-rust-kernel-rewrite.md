# SimpleKernel Rust Rewrite — Complete Migration Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Rewrite SimpleKernel from C++23/C23 to Rust, preserving the interface-driven educational design philosophy, dual-architecture support (riscv64/aarch64), and all existing functionality.

**Architecture:** Incremental migration in 7 phases, from boot foundation → arch abstraction → memory → interrupts → tasks → devices → filesystem. Each phase produces a bootable (or testable) artifact. Assembly files (.S) and linker scripts (.ld) are reused. The Rust kernel follows `#![no_std]` + `#![no_main]` conventions with a custom target specification per architecture.

**Tech Stack:** Rust nightly (for `asm!`, `naked_fn`, `alloc_error_handler`, custom allocators), Cargo workspace, `build.rs` for linker script integration + assembly compilation via `cc` crate, U-Boot FIT image packaging via `xtask`, QEMU for testing. Key crates: `spin` (locks), `bitflags` (register fields), `buddy_system_allocator` (heap), `fdt` (device tree), `log` (logging facade).

**Boot Model (CRITICAL):** The current kernel boots through a full firmware chain — NOT raw QEMU `-bios none -kernel`. The Rust rewrite MUST preserve this:
- **riscv64:** U-Boot SPL → OpenSBI → U-Boot → TFTP loads `boot.fit` (FIT image containing kernel ELF + DTB) → kernel `_start`
- **aarch64:** U-Boot → ATF → OP-TEE → U-Boot → TFTP loads `boot.fit` → kernel `_start`
- QEMU flags include: `-nographic -serial stdio -m 1024M -smp 2`, VirtIO MMIO devices, rootfs.img (FAT32, 64MB)
- The `xtask` build tool must replicate: `mkimage -f boot.its boot.fit`, `mkimage -T script` for boot.scr, DTB generation via `qemu-system-* -machine dumpdtb=`, and TFTP symlinks to `/srv/tftp/`

---

## Table of Contents

1. [Current Codebase Inventory](#1-current-codebase-inventory)
2. [C++ → Rust Pattern Mapping](#2-c--rust-pattern-mapping)
3. [Third-Party Dependency Replacement](#3-third-party-dependency-replacement)
4. [Project Structure (Rust)](#4-project-structure-rust)
5. [Build System Design](#5-build-system-design)
6. [Migration Phases](#6-migration-phases)
7. [Testing Strategy](#7-testing-strategy)
8. [Risk Assessment & Mitigation](#8-risk-assessment--mitigation)
9. [Preserving Interface-Driven Philosophy](#9-preserving-interface-driven-philosophy)

---

## 1. Current Codebase Inventory

### 1.1 Code Metrics

| Subsystem | Lines | Files | Key Components |
|-----------|------:|------:|----------------|
| `src/arch` | 3,841 | 28+ | boot.S, arch_main, interrupt.S/cpp, switch.S, timer, syscall, backtrace, link.ld (×2 arch) |
| `src/device` | 4,630 | 30+ | DeviceManager, DriverRegistry, PlatformBus, NS16550A, PL011, VirtIO (blk/console/gpu/input/net) |
| `src/filesystem` | 4,106 | 18+ | VFS (open/close/read/write/mkdir/rmdir/seek/lookup/mount/readdir/unlink), RamFS, FatFS |
| `src/task` | 3,696 | 16 | TaskManager, TCB, CFS/FIFO/RR schedulers, Mutex, Signal, Clone/Exit/Wait/Sleep/Block/Wakeup/Balance |
| `src/memory` | 646 | 4 | VirtualMemory (MapPage/UnmapPage/GetMapping/Clone/Destroy) |
| `src/include` | 2,936 | 17 | All public interface headers (SpinLock, Expected, KernelLog, PerCpu, etc.) |
| `src/libc` | 703 | 5 | sk_string, sk_stdlib, sk_ctype, sk_stdio |
| `src/libcxx` | 380 | 4 | C++ runtime (new/delete, __cxa_*) |
| **Total kernel** | **~21,459** | **~144** | |
| `tests/` | ~33,969 | 21+ | Unit (GoogleTest), integration, system (QEMU) |

### 1.2 Inventory Details Often Missed

The following components are easy to overlook but MUST be ported:

| Component | C++ File | Rust Target | Notes |
|-----------|----------|-------------|-------|
| **ACPI tables** | `src/device/acpi/acpi.hpp`, `acpi_driver.hpp` | `kernel/src/device/acpi.rs` | Table structures (RSDP, XSDT, FADT, DSDT) + driver |
| **Task FSM messages** | `src/task/include/task_messages.hpp` | `kernel/src/task/messages.rs` | 9 message types (Schedule, Yield, Sleep, Block, Wakeup, Exit, Reap, Stop, Cont) — uses ETL message framework, port as Rust enum |
| **Lifecycle messages** | `src/task/include/lifecycle_messages.hpp` | `kernel/src/task/messages.rs` | ThreadCreate, ThreadExit messages with PID/exit_code |
| **Resource ID** | `src/task/include/resource_id.hpp` | `kernel/src/task/resource_id.rs` | Typed resource ID (8-bit type + 56-bit data), 10 resource types (Mutex, Semaphore, CondVar, ChildExit, IoComplete, Futex, Signal, Timer, Interrupt), Hash impl |
| **Scheduler hooks** | `scheduler_base.hpp` lines 88-132 | `kernel/src/task/scheduler/mod.rs` | `OnTimeSliceExpired`, `BoostPriority`, `RestorePriority`, `OnPreempted`, `OnScheduled`, `GetStats`, `ResetStats` — all must be in trait |
| **Logging queue** | `src/include/kernel_log.hpp` | `kernel/src/logging.rs` | MPMC queue-based (256 entries × 256 bytes = 64KB static), per-entry has seq/core_id/level/msg[239], non-blocking drain with atomic_flag try-lock, compile-time level filtering, drop counting |
| **Message router IDs** | `task_messages.hpp` | `kernel/src/task/messages.rs` | Router IDs for TimerHandler, TaskFsm, VirtioBlk, VirtioNet |
| **VirtIO device stubs** | `src/device/virtio/device/{console,gpu,input,net}/` | `kernel/src/device/virtio/{console,gpu,input,net}.rs` | Currently header-only stubs with type defs |

### 1.3 Architecture-Specific Files (Per Arch)

Each architecture (riscv64, aarch64) has a mirrored file set:

| File | Purpose | Rust Strategy |
|------|---------|---------------|
| `boot.S` | Entry point, stack setup | **Reuse** (assembly, minimal changes) |
| `link.ld` | Memory layout, sections | **Reuse** (linker scripts are language-agnostic) |
| `arch_main.cpp` | ArchInit: FDT, console, SMP | Rewrite in Rust |
| `early_console.cpp` | Pre-driver output (SBI/PL011) | Rewrite in Rust |
| `interrupt.S` | Trap vector table | **Reuse** (assembly) |
| `interrupt_main.cpp` | Interrupt dispatch | Rewrite in Rust |
| `interrupt.cpp` | Interrupt controller (PLIC/GIC) | Rewrite in Rust |
| `timer.cpp` | Timer init, tick handler | Rewrite in Rust |
| `switch.S` | Context switch registers | **Reuse** (assembly) |
| `syscall.cpp` | System call handling | Rewrite in Rust |
| `backtrace.cpp` | Stack unwinding | Rewrite in Rust |
| `macro.S` | Assembly macros | **Reuse** |
| `plic/plic.cpp` or `gic/gic.cpp` | Interrupt controller driver | Rewrite in Rust |

### 1.3 Boot Sequence (Must Be Preserved)

```
boot.S (_start entry)
  └→ _start() [Rust: #[no_mangle] extern "C"]
       ├→ Primary core:
       │    CppInit() → [Rust: no equivalent needed, global_alloc setup instead]
       │    main()
       │      ├→ PerCpuArray::create()
       │      ├→ ArchInit()         — Phase 1
       │      ├→ MemoryInit()       — Phase 3
       │      ├→ InterruptInit()    — Phase 4
       │      ├→ DeviceInit()       — Phase 6
       │      ├→ FileSystemInit()   — Phase 7
       │      ├→ TaskManager::create() + InitCurrentCore(primary=true) — Phase 5
       │      ├→ TimerInit()        — Phase 4
       │      ├→ WakeUpOtherCores()
       │      └→ Schedule() loop
       └→ Secondary cores:
            main_smp()
              ├→ ArchInitSMP()
              ├→ MemoryInitSMP()
              ├→ InterruptInitSMP()
              ├→ TaskManager::InitCurrentCore()
              ├→ TimerInitSMP()
              └→ Schedule()
```

---

## 2. C++ → Rust Pattern Mapping

### 2.1 Type System & Error Handling

| C++ Pattern | Rust Equivalent | Notes |
|-------------|-----------------|-------|
| `std::expected<T, Error>` | `Result<T, KernelError>` | Direct mapping, Rust native |
| `ErrorCode` enum | `#[repr(u64)] enum ErrorCode` | Keep numeric codes for ABI compat |
| `Error` struct | `struct KernelError { code: ErrorCode }` | Implement `core::fmt::Display` |
| `Expected<T>` alias | `type KResult<T> = Result<T, KernelError>;` | Project-wide alias |
| `void` return with error | `Result<(), KernelError>` | |
| `nullptr` check | `Option<T>` | Rust eliminates null entirely |

### 2.2 OOP & Polymorphism

| C++ Pattern | Rust Equivalent | Notes |
|-------------|-----------------|-------|
| Abstract base class (ABC) | `trait` | `SchedulerBase` → `trait Scheduler` |
| Virtual dispatch (`virtual`) | `dyn Trait` (trait object) | For runtime polymorphism |
| Static dispatch (templates) | Generics `<T: Trait>` | For compile-time polymorphism |
| `class` with private fields | `struct` + `pub`/private fields | Rust module = visibility boundary |
| Inheritance | Trait composition | No struct inheritance in Rust |
| `const` member function | `&self` method | Immutable borrow |
| `virtual ~Destructor()` | `Drop` trait | Automatic in Rust |

### 2.3 Concurrency & Synchronization

| C++ Pattern | Rust Equivalent | Notes |
|-------------|-----------------|-------|
| `SpinLock` (custom, interrupt-aware) | Custom `SpinLock<T>` wrapping data (saves/restores interrupt state on lock/unlock, tracks owner core, supports lock-level ordering) | `spin::Mutex` is NOT sufficient — current SpinLock disables interrupts and has lock-level hierarchy. Must write custom. |
| `LockGuard<SpinLock>` | Custom `SpinLockGuard<'_, T>` (re-enables interrupts on drop) | RAII, restores interrupt state on guard drop |
| `std::atomic<T>` | `core::sync::atomic::AtomicT` | Same semantics |
| `std::atomic_flag` | `AtomicBool` | |
| `Mutex` (blocking) | Custom `Mutex` with task blocking | Need custom implementation like C++ version |

### 2.4 Memory & Ownership

| C++ Pattern | Rust Equivalent | Notes |
|-------------|-----------------|-------|
| `etl::unique_ptr<T>` | `Box<T>` (with `#[global_allocator]`) | Ownership transfer via move |
| `new`/`delete` | `Box::new()` / auto-drop | |
| `aligned_alloc`/`aligned_free` | `GlobalAlloc` trait impl | Wraps bmalloc or Rust allocator |
| Raw pointer `T*` | `*const T` / `*mut T` inside `unsafe` | Minimize unsafe surface |
| RAII | Ownership + `Drop` | Rust's natural model |
| `etl::vector<T, N>` | `heapless::Vec<T, N>` or `ArrayVec` | No-alloc fixed-capacity |
| `etl::unordered_map<K,V,N>` | `heapless::FnvIndexMap` or custom | No-alloc fixed-capacity |
| `etl::list<T, N>` | `heapless::Deque` or intrusive list | |

### 2.5 Singleton Pattern

| C++ Pattern | Rust Equivalent | Notes |
|-------------|-----------------|-------|
| `etl::singleton<T>` | `static INSTANCE: spin::Once<T>` | Initialize with `.call_once()` |
| `TaskManagerSingleton::instance()` | `TASK_MANAGER.get().unwrap()` | Or `get_unchecked()` after init |
| `TaskManagerSingleton::create()` | `TASK_MANAGER.call_once(\|\| TaskManager::new())` | One-time init |

### 2.6 Assembly & Low-Level

| C++ Pattern | Rust Equivalent | Notes |
|-------------|-----------------|-------|
| `extern "C"` function | `#[no_mangle] extern "C" fn` | FFI compatible |
| Inline assembly (`asm volatile`) | `core::arch::asm!()` | Rust nightly feature |
| `__always_inline` | `#[inline(always)]` | |
| `[[noreturn]]` | `-> !` (never type) | |
| `[[maybe_unused]]` | `_` prefix or `#[allow(unused)]` | |
| `[[nodiscard]]` | `#[must_use]` | |
| `__builtin_unreachable()` | `core::hint::unreachable_unchecked()` | Inside `unsafe` |
| `static_assert` | `const_assert!` or `const { assert!(...) }` | Compile-time check |

### 2.7 Preprocessor & Build Config

| C++ Pattern | Rust Equivalent | Notes |
|-------------|-----------------|-------|
| `#ifdef RISCV64` | `#[cfg(target_arch = "riscv64")]` | Cargo conditional compilation |
| `#pragma once` | Module system (no header guards) | |
| `#include` | `use` / `mod` | |
| `CMAKE_SYSTEM_PROCESSOR` | Cargo target triple | `riscv64gc-unknown-none-elf` etc. |
| Macro `#define` | `macro_rules!` or `const` | |

---

## 3. Third-Party Dependency Replacement

### 3.1 Direct Replacements (Rust Crates)

| C++ Dependency | Rust Replacement | Crate | Notes |
|----------------|------------------|-------|-------|
| `googletest` | Built-in `#[test]` + custom `no_std` harness | — | QEMU test runner for on-target |
| `nanoprintf` | `core::fmt` + `write!` macro | — | Built into Rust |
| `cpu_io` | Inline `asm!` + arch-specific crates | `riscv`, `aarch64-cpu` | Or custom thin wrappers |
| `opensbi` / `opensbi_interface` | `sbi-rt` / `sbi-spec` | `sbi-rt` | Standard RISC-V SBI interface |
| `etl` (Embedded Template Library) | `heapless` + `spin` | `heapless`, `spin` | Fixed-capacity collections + locks |
| `bmalloc` | `buddy_system_allocator` | `buddy_system_allocator` | Better for page-granularity allocation; implement `GlobalAlloc` trait |
| `MPMCQueue` | Custom `no_std` ring buffer (port current design) | — (custom) | `heapless::mpmc` is bounded to small sizes; current design uses 256-entry × 256-byte entries for logging. Write a minimal `StaticMpMcQueue<T, N>` matching current semantics. |
| `fatfs` | Keep C `fatfs` library via FFI bindgen | `bindgen` (build-dep) | Pure Rust `fatfs` crate requires `std::io` traits; for no_std kernel, wrapping the existing C fatfs via FFI is more reliable. Alternative: port the `diskio` layer to Rust and keep fatfs C code. |
| `EasyLogger` | `log` facade + custom `no_std` backend | `log` | Standard logging in Rust |
| `dtc` / FDT parsing | `fdt` crate | `fdt` | Pure Rust FDT parser |

### 3.2 Firmware (Unchanged — Not Part of Rewrite)

These are external firmware binaries, not kernel code. They remain as-is:

| Component | Status |
|-----------|--------|
| `u-boot` | Keep (bootloader, builds separately) |
| `opensbi` | Keep (RISC-V SBI firmware) |
| `arm-trusted-firmware` | Keep (AArch64 ATF) |
| `optee` | Keep (OP-TEE) |
| `dtc` | Keep (build tool for device tree compilation) |

### 3.3 No Longer Needed (Rust Provides Natively)

| C++ Component | Why Not Needed |
|---------------|----------------|
| `src/libc/` (sk_string, sk_stdlib, sk_ctype) | `core::str`, `core::slice`, `core::char` |
| `src/libcxx/` (new/delete, __cxa_*) | Rust runtime handles allocation, no C++ ABI needed |
| `nanoprintf` | `core::fmt::Write` trait |
| ETL containers | `heapless` crate or Rust `alloc` |
| Header guard boilerplate | Rust module system |

---

## 4. Project Structure (Rust)

```
SimpleKernel/
├── Cargo.toml                  # Workspace root
├── rust-toolchain.toml         # Pin nightly version
├── .cargo/
│   └── config.toml             # Per-target rustflags, linker, runner
├── targets/
│   ├── riscv64gc-simplekernel.json    # Custom target spec (riscv64)
│   └── aarch64-simplekernel.json      # Custom target spec (aarch64)
├── kernel/                     # Main kernel crate
│   ├── Cargo.toml
│   ├── build.rs                # Linker script selection, assembly compilation
│   ├── src/
│   │   ├── main.rs             # #![no_std] #![no_main] entry
│   │   ├── lang_items.rs       # panic_handler, alloc_error_handler
│   │   ├── arch/
│   │   │   ├── mod.rs          # cfg-gated arch module selection
│   │   │   ├── riscv64/
│   │   │   │   ├── mod.rs
│   │   │   │   ├── boot.S      # [REUSED] Assembly entry point
│   │   │   │   ├── link.ld     # [REUSED] Linker script
│   │   │   │   ├── switch.S    # [REUSED] Context switch
│   │   │   │   ├── interrupt.S # [REUSED] Trap vector
│   │   │   │   ├── macro.S     # [REUSED] Assembly macros
│   │   │   │   ├── init.rs     # ArchInit, ArchInitSMP
│   │   │   │   ├── console.rs  # Early console (SBI)
│   │   │   │   ├── interrupt.rs# PLIC + interrupt dispatch
│   │   │   │   ├── timer.rs    # SBI timer
│   │   │   │   ├── syscall.rs  # ecall handling
│   │   │   │   ├── context.rs  # InitTaskContext, TrapContext
│   │   │   │   └── backtrace.rs
│   │   │   └── aarch64/
│   │   │       ├── mod.rs
│   │   │       ├── boot.S      # [REUSED]
│   │   │       ├── link.ld     # [REUSED]
│   │   │       ├── switch.S    # [REUSED]
│   │   │       ├── interrupt.S # [REUSED]
│   │   │       ├── macro.S     # [REUSED]
│   │   │       ├── init.rs     # ArchInit, ArchInitSMP
│   │   │       ├── console.rs  # Early console (PL011 MMIO)
│   │   │       ├── interrupt.rs# GICv3 + interrupt dispatch
│   │   │       ├── timer.rs    # Generic timer
│   │   │       ├── syscall.rs  # svc handling
│   │   │       ├── context.rs  # InitTaskContext, TrapContext
│   │   │       └── backtrace.rs
│   │   ├── memory/
│   │   │   ├── mod.rs
│   │   │   ├── virtual_memory.rs    # PageTable ops
│   │   │   ├── physical_memory.rs   # Frame allocator
│   │   │   ├── heap.rs              # #[global_allocator] wrapping buddy_system_allocator
│   │   │   └── address.rs           # PhysAddr/VirtAddr types
│   │   ├── task/
│   │   │   ├── mod.rs
│   │   │   ├── manager.rs           # TaskManager
│   │   │   ├── tcb.rs               # TaskControlBlock
│   │   │   ├── resource_id.rs       # ResourceType + ResourceId (type-tagged 64-bit ID)
│   │   │   ├── messages.rs          # Task FSM messages (9 types) + lifecycle messages
│   │   │   ├── scheduler/
│   │   │   │   ├── mod.rs           # Scheduler trait + SchedulerStats
│   │   │   │   ├── cfs.rs
│   │   │   │   ├── fifo.rs
│   │   │   │   └── round_robin.rs
│   │   │   ├── fsm.rs               # Task state machine
│   │   │   ├── clone.rs
│   │   │   ├── exit.rs
│   │   │   ├── wait.rs
│   │   │   ├── sleep.rs
│   │   │   ├── block.rs
│   │   │   ├── wakeup.rs
│   │   │   ├── balance.rs
│   │   │   ├── signal.rs
│   │   │   └── mutex.rs
│   │   ├── device/
│   │   │   ├── mod.rs
│   │   │   ├── manager.rs           # DeviceManager
│   │   │   ├── registry.rs          # DriverRegistry
│   │   │   ├── platform_bus.rs      # FDT-based device enumeration
│   │   │   ├── node.rs              # DeviceNode
│   │   │   ├── acpi.rs              # ACPI table structures (RSDP, XSDT, FADT, DSDT)
│   │   │   ├── ns16550a.rs          # UART driver
│   │   │   ├── pl011.rs             # PL011 driver
│   │   │   └── virtio/
│   │   │       ├── mod.rs
│   │   │       ├── transport.rs     # MMIO transport
│   │   │       ├── queue.rs         # Virtqueue (split)
│   │   │       ├── blk.rs           # Block device
│   │   │       ├── console.rs
│   │   │       ├── gpu.rs
│   │   │       ├── input.rs
│   │   │       └── net.rs
│   │   ├── fs/
│   │   │   ├── mod.rs
│   │   │   ├── vfs.rs               # VFS layer
│   │   │   ├── file_descriptor.rs
│   │   │   ├── ramfs.rs
│   │   │   └── fatfs.rs
│   │   ├── sync/
│   │   │   ├── mod.rs
│   │   │   ├── spinlock.rs
│   │   │   └── mutex.rs
│   │   ├── logging.rs               # klog replacement
│   │   ├── error.rs                 # KernelError, ErrorCode, KResult<T>
│   │   ├── config.rs                # kernel_config constants
│   │   ├── per_cpu.rs               # Per-CPU data
│   │   ├── fdt.rs                   # Device tree wrapper
│   │   ├── elf.rs                   # ELF parser
│   │   ├── io_buffer.rs
│   │   ├── mmio.rs                  # MMIO accessor
│   │   ├── panic.rs                 # Panic handler + observers
│   │   └── syscall.rs               # Syscall dispatch table
│   └── tests/                       # Integration tests (host)
├── xtask/                      # Build helper (replaces cmake/functions.cmake)
│   ├── Cargo.toml
│   └── src/main.rs             # cargo xtask run, cargo xtask debug, etc.
└── tests/                      # Preserved for system tests compatibility
    ├── unit_test/              # Migrated to Rust #[test]
    └── system_test/            # QEMU-based tests
```

---

## 5. Build System Design

### 5.1 Cargo Workspace

```toml
# Cargo.toml (workspace root)
[workspace]
members = ["kernel", "xtask"]
resolver = "2"

[profile.dev]
panic = "abort"
opt-level = 1        # Kernel needs some optimization even in debug

[profile.release]
panic = "abort"
lto = true
codegen-units = 1
```

### 5.2 Kernel Crate

```toml
# kernel/Cargo.toml
[package]
name = "simplekernel"
version = "0.1.0"
edition = "2024"

[dependencies]
# Synchronization (custom SpinLock is primary, spin used for Once/Lazy only)
spin = { version = "0.9", default-features = false, features = ["once"] }
bitflags = "2"
log = { version = "0.4", default-features = false }
# Fixed-capacity collections for hot paths (no heap allocation)
heapless = "0.8"
# Heap allocator (buddy system for page-granularity)
buddy_system_allocator = "0.11"
# Device tree parser (pure Rust, no_std)
fdt = "0.2"

# Architecture-specific
[target.'cfg(target_arch = "riscv64")'.dependencies]
sbi-rt = { version = "0.0.3", features = ["legacy"] }
riscv = { version = "0.12", features = ["critical-section-single-hart"] }

[target.'cfg(target_arch = "aarch64")'.dependencies]
aarch64-cpu = "9"
tock-registers = "0.9"

[build-dependencies]
# REQUIRED: compiles assembly (.S) files that are preprocessed by GCC
cc = "1"
```

### 5.3 Custom Target Specs

```json
// targets/riscv64gc-simplekernel.json
{
  "llvm-target": "riscv64gc-unknown-none-elf",
  "data-layout": "e-m:e-p:64:64-i64:64-i128:128-n32:64-S128",
  "arch": "riscv64",
  "target-endian": "little",
  "target-pointer-width": "64",
  "target-c-int-width": "32",
  "os": "none",
  "executables": true,
  "linker-flavor": "ld.lld",
  "linker": "rust-lld",
  "panic-strategy": "abort",
  "disable-redzone": true,
  "features": "+m,+a,+f,+d,+c",
  "code-model": "medium",
  "pre-link-args": {}
}
// NOTE: Linker script is set ONLY in build.rs via cargo:rustc-link-arg.
// Do NOT also set it in .cargo/config.toml or target JSON pre-link-args.
```

```json
// targets/aarch64-simplekernel.json
{
  "llvm-target": "aarch64-unknown-none",
  "data-layout": "e-m:e-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128",
  "arch": "aarch64",
  "target-endian": "little",
  "target-pointer-width": "64",
  "target-c-int-width": "32",
  "os": "none",
  "executables": true,
  "linker-flavor": "ld.lld",
  "linker": "rust-lld",
  "panic-strategy": "abort",
  "disable-redzone": true,
  "features": "+v8a",
  "pre-link-args": {}
}
// NOTE: Linker script is set ONLY in build.rs via cargo:rustc-link-arg.
// Do NOT also set it in .cargo/config.toml or target JSON pre-link-args.
```

### 5.4 Cargo Config

```toml
# .cargo/config.toml

# CRITICAL: build-std is required for custom no_std targets
[unstable]
build-std = ["core", "compiler_builtins", "alloc"]
build-std-features = ["compiler-builtins-mem"]

[build]
# Default target (override per-invocation)
# target = "targets/riscv64gc-simplekernel.json"

# NOTE: No "runner" config here — QEMU boot requires U-Boot FIT images,
# which cannot be expressed as a simple runner command.
# Use `cargo xtask run --arch riscv64` instead, which handles:
# 1. Build kernel ELF
# 2. Generate DTB via qemu-system-* -machine dumpdtb=
# 3. Create FIT image (mkimage -f boot.its boot.fit)
# 4. Create boot script (mkimage -T script)
# 5. Setup TFTP symlinks
# 6. Launch QEMU with full firmware chain

# NOTE: Linker script is set ONLY in build.rs via cargo:rustc-link-arg.
# Do NOT duplicate it here in rustflags — that causes double-linking errors.
# If per-target rustflags are needed for other purposes, add them below:
# [target.'cfg(target_arch = "riscv64")']
# rustflags = []
# [target.'cfg(target_arch = "aarch64")']
# rustflags = []

[alias]
xtask = "run --package xtask --"
```

### 5.5 build.rs (Assembly Compilation)

```rust
// kernel/build.rs
use std::path::PathBuf;

fn main() {
    let arch = std::env::var("CARGO_CFG_TARGET_ARCH").unwrap();
    let arch_dir = PathBuf::from("src/arch").join(&arch);

    // Assembly files per architecture (preprocessed .S, compiled by cross-GCC)
    let asm_files: Vec<&str> = match arch.as_str() {
        "riscv64" => vec!["boot.S", "switch.S", "interrupt.S", "macro.S"],
        "aarch64" => vec!["boot.S", "switch.S", "interrupt.S", "macro.S"],
        _ => panic!("Unsupported architecture: {}", arch),
    };

    // Use cross-compiler for assembly (must match target arch)
    let compiler = match arch.as_str() {
        "riscv64" => "riscv64-linux-gnu-gcc",
        "aarch64" => "aarch64-linux-gnu-gcc",
        _ => unreachable!(),
    };

    let mut build = cc::Build::new();
    build.compiler(compiler);

    // Pass preprocessor defines that assembly files expect
    // These match the CMake cache variables from CMakePresets.json
    build.define("SIMPLEKERNEL_MAX_CORE_COUNT", "4");
    build.define("SIMPLEKERNEL_DEFAULT_STACK_SIZE", "16384");
    if arch == "riscv64" {
        build.define("SIMPLEKERNEL_EARLY_CONSOLE_BASE", "0x10000000");
        build.define("SIMPLEKERNEL_PER_CPU_ALIGN_SIZE", "128");
    } else {
        build.define("SIMPLEKERNEL_EARLY_CONSOLE_BASE", "0x9000000");
        build.define("SIMPLEKERNEL_PER_CPU_ALIGN_SIZE", "128");
    }

    // Include paths for assembly macros
    build.include(&arch_dir);

    for file in &asm_files {
        build.file(arch_dir.join(file));
    }
    build.compile("asm");

    // Link the linker script
    println!("cargo:rustc-link-arg=-T{}", arch_dir.join("link.ld").display());

    // Re-run triggers
    println!("cargo:rerun-if-changed=src/arch/{}/link.ld", arch);
    for file in &asm_files {
        println!("cargo:rerun-if-changed=src/arch/{}/{}", arch, file);
    }
}
```

**Entry point contract (`boot.S` → Rust `_start`):**
The assembly `boot.S` sets up the initial stack and jumps to `_start`. The Rust entry must be:
```rust
// kernel/src/main.rs
#[no_mangle]
pub extern "C" fn _start(argc: i32, argv: *const *const u8) -> ! {
    // argc: riscv64 = boot hart ID; aarch64 = 0
    // argv: riscv64 = DTB address; aarch64 = DTB address
    // ...
}
```
The `_start` symbol must appear in the linker script's ENTRY directive. Verify with `objdump -t kernel.elf | grep _start`.

### 5.6 xtask (Build Commands)

The `xtask` crate replaces CMake's `make run`, `make debug`, etc:

```
cargo xtask build [--arch riscv64|aarch64]    # Build kernel
cargo xtask run [--arch riscv64|aarch64]      # Run in QEMU
cargo xtask debug [--arch riscv64|aarch64]    # GDB debug in QEMU
cargo xtask test                               # Run all tests
cargo xtask firmware [--arch ...]              # Build U-Boot/OpenSBI/ATF
```

---

## 6. Migration Phases

### Runnable Milestone Principle (关键原则)

**每个阶段结束时，内核必须能在 QEMU 中运行并产生可观察的输出。**

| Phase | 运行后你会看到什么 | 你可以修改什么 |
|-------|-------------------|---------------|
| P0 | 内核启动后停在死循环（QEMU 无输出，不崩溃） | 链接脚本、boot.S 入口、Cargo 配置 |
| P1 | 串口打印 "Hello SimpleKernel"，显示 FDT 解析结果 | 日志格式、FDT 解析逻辑、控制台驱动 |
| P2 | 同 P1，但 panic 时显示调用栈，SpinLock 可用 | SpinLock 策略、ELF 解析、panic 格式 |
| P3 | 同 P2，内核启用分页后继续运行，堆分配可用 | 页表结构、帧分配器、堆大小 |
| P4 | 定时器中断触发日志输出（每秒 ~1000 次 tick） | 中断控制器配置、定时器频率、syscall 分发 |
| P5 | 多个内核线程在运行，调度器选择任务，idle 线程让出 CPU | 调度策略、TCB 结构、clone/exit 逻辑 |
| P6 | 设备从 FDT 枚举，UART 通过驱动框架输出，VirtIO 块设备读写 | 驱动匹配表、设备框架、VirtIO 协议 |
| P7 | 文件系统挂载，可以创建/读取/删除文件 | VFS 接口、RamFS 实现、FatFS 适配 |

**每个阶段的验证流程（你必须执行）：**
```bash
# 1. 编译
cargo build --target targets/riscv64gc-simplekernel.json

# 2. 运行（通过 xtask，处理 U-Boot FIT 打包）
cargo xtask run --arch riscv64

# 3. 观察串口输出，对照该阶段的 "Expected Output"

# 4. 退出 QEMU: Ctrl+A, X
```

**你在每个阶段后可以自由修改任何已完成的文件，只需确保：**
1. `cargo build` 通过
2. `cargo xtask run` 后串口输出符合该阶段的预期
3. `cargo test` 单元测试通过（如果有）

---

### Overview

```
Phase 0: Project Skeleton & Build System         [1 week]
Phase 1: Boot & Early Console (per arch)         [2 weeks]
Phase 2: Core Infrastructure                      [1 week]
Phase 3: Memory Management                        [2 weeks]
Phase 4: Interrupt & Timer                        [2 weeks]
Phase 5: Task Management & Scheduling             [3 weeks]
Phase 6: Device Framework & Drivers               [3 weeks]
Phase 7: Filesystem                               [2 weeks]
                                            Total: ~16 weeks
```

### Dependency Graph

```
Phase 0 ──→ Phase 1 ──→ Phase 2 ──┬──→ Phase 3 ──→ Phase 4 ──→ Phase 5
                                   │                                │
                                   │                                ├──→ Phase 6
                                   │                                │
                                   │                                └──→ Phase 7
                                   │                                       │
                                   │                                       ▼
                                   └──────────────────────────────→ [Complete]
```

Note: Phase 6 and 7 can be parallelized after Phase 5 is complete.

---

### Phase 0: Project Skeleton & Build System

**Goal:** Empty Rust kernel that compiles for both architectures, boots in QEMU, and halts.

**Files to create:**
- `Cargo.toml` (workspace)
- `kernel/Cargo.toml`
- `kernel/build.rs`
- `kernel/src/main.rs` (minimal `#![no_std]`, `#![no_main]`, `_start` → infinite loop)
- `kernel/src/lang_items.rs` (`panic_handler`)
- `kernel/src/arch/mod.rs`
- `kernel/src/arch/riscv64/mod.rs` (empty)
- `kernel/src/arch/aarch64/mod.rs` (empty)
- `targets/riscv64gc-simplekernel.json`
- `targets/aarch64-simplekernel.json`
- `.cargo/config.toml`
- `rust-toolchain.toml`
- `xtask/Cargo.toml` + `xtask/src/main.rs`

**Files to reuse (copy from C++ project):**
- `src/arch/riscv64/boot.S` → `kernel/src/arch/riscv64/boot.S`
- `src/arch/riscv64/link.ld` → `kernel/src/arch/riscv64/link.ld`
- `src/arch/aarch64/boot.S` → `kernel/src/arch/aarch64/boot.S`
- `src/arch/aarch64/link.ld` → `kernel/src/arch/aarch64/link.ld`

**Steps:**
- [ ] Step 1: Create workspace Cargo.toml and kernel crate skeleton
- [ ] Step 2: Create custom target JSON specs for both architectures
- [ ] Step 3: Create `.cargo/config.toml` with target-specific rustflags
- [ ] Step 4: Create `rust-toolchain.toml` pinning nightly + `rust-src` component
- [ ] Step 5: Write minimal `main.rs` with `#![no_std]`, `#![no_main]`, `_start` → loop
- [ ] Step 6: Write `lang_items.rs` with `#[panic_handler]`
- [ ] Step 7: Copy and adapt boot.S files (adjust `_start` symbol expectations)
- [ ] Step 8: Copy linker scripts, verify section names match Rust output
- [ ] Step 9: Write `build.rs` for assembly compilation
- [ ] Step 10: Build for riscv64: `cargo build --target targets/riscv64gc-simplekernel.json`
- [ ] Step 11: Build for aarch64: `cargo build --target targets/aarch64-simplekernel.json`
- [ ] Step 12: Boot in QEMU (expect hang in infinite loop — success = no crash)
- [ ] Step 13: Create xtask crate with `run` and `build` subcommands
- [ ] Step 14: Commit: `feat(build): add Rust project skeleton with dual-arch support`

**Expected QEMU Output (riscv64):**
```
U-Boot 20XX.XX (...)
...
## Loading kernel from FIT Image ...
   Using 'conf-1' configuration
   ...
   Booting using the fdt blob at ...
(无更多输出，内核在死循环中 — 这就是成功)
```
按 Ctrl+A, X 退出 QEMU。

**验证命令：**
```bash
# 编译
cargo build --target targets/riscv64gc-simplekernel.json
# 检查 ELF 结构
riscv64-linux-gnu-objdump -h target/riscv64gc-simplekernel/debug/simplekernel | grep -E '\.text|\.rodata|\.data|\.bss'
riscv64-linux-gnu-objdump -t target/riscv64gc-simplekernel/debug/simplekernel | grep _start
# 运行
cargo xtask run --arch riscv64
```

**Exit Criteria:**
1. `cargo build` 两个架构都 exit 0
2. `objdump -h` 显示 `.text`, `.rodata`, `.data`, `.bss` 段在正确地址
3. `objdump -t | grep _start` 显示入口符号
4. QEMU 启动不崩溃（U-Boot 加载后内核进入死循环）

**你可以修改的内容：**
- `link.ld` — 调整内存布局、段地址
- `boot.S` — 修改栈大小、入口逻辑
- `Cargo.toml` — 添加/移除依赖
- `targets/*.json` — 调整 CPU feature flags

---

### Phase 1: Boot & Early Console

**Goal:** Kernel boots, parses FDT, prints "Hello SimpleKernel" to UART on both architectures.

**Depends on:** Phase 0

**Files to create:**
- `kernel/src/arch/riscv64/init.rs` — `ArchInit()` (FDT parse, early console)
- `kernel/src/arch/riscv64/console.rs` — SBI putchar
- `kernel/src/arch/aarch64/init.rs` — `ArchInit()` (FDT parse, early console)
- `kernel/src/arch/aarch64/console.rs` — PL011 MMIO putchar
- `kernel/src/logging.rs` — `klog` replacement: MPMC queue-based (256 entries × 256 bytes = 64KB static), non-blocking drain with atomic try-lock, per-entry {seq, core_id, level, msg[239]}, compile-time level filtering, drop counting, `RawPut` for panic paths
- `kernel/src/fdt.rs` — FDT wrapper using `fdt` crate
- `kernel/src/error.rs` — `ErrorCode`, `KernelError`, `KResult<T>`
- `kernel/src/config.rs` — Constants (max cores, etc.)
- `kernel/src/per_cpu.rs` — Per-CPU data structures
- `kernel/src/mmio.rs` — MMIO register access helper

**C++ reference files to port:**
- `src/arch/riscv64/arch_main.cpp` → `kernel/src/arch/riscv64/init.rs`
- `src/arch/riscv64/early_console.cpp` → `kernel/src/arch/riscv64/console.rs`
- `src/arch/aarch64/arch_main.cpp` → `kernel/src/arch/aarch64/init.rs`
- `src/arch/aarch64/early_console.cpp` → `kernel/src/arch/aarch64/console.rs`
- `src/include/expected.hpp` → `kernel/src/error.rs`
- `src/include/kernel_fdt.hpp` → `kernel/src/fdt.rs`
- `src/include/kernel_log.hpp` → `kernel/src/logging.rs`
- `src/include/per_cpu.hpp` → `kernel/src/per_cpu.rs`
- `src/include/kernel_config.hpp` → `kernel/src/config.rs`
- `src/include/mmio_accessor.hpp` → `kernel/src/mmio.rs`
- `src/include/basic_info.hpp` → integrated into `per_cpu.rs`

**Steps:**
- [ ] Step 1: Implement `error.rs` — `ErrorCode` enum, `KernelError`, `KResult<T>`
- [ ] Step 2: Implement `config.rs` — port constants from `kernel_config.hpp`
- [ ] Step 3: Implement `mmio.rs` — volatile MMIO read/write helpers
- [ ] Step 4: Implement `logging.rs` — `log` crate backend with UART output
- [ ] Step 5: Implement early console for riscv64 (SBI ecall for putchar)
- [ ] Step 6: Implement early console for aarch64 (PL011 MMIO)
- [ ] Step 7: Implement `fdt.rs` — thin wrapper around `fdt` crate
- [ ] Step 8: Implement `per_cpu.rs` — PerCpu struct, array, accessor
- [ ] Step 9: Implement `arch/riscv64/init.rs` — ArchInit (parse FDT, setup console, SMP data)
- [ ] Step 10: Implement `arch/aarch64/init.rs` — ArchInit (parse FDT, setup console, SMP data)
- [ ] Step 11: Wire `main.rs` to call ArchInit and print "Hello SimpleKernel"
- [ ] Step 12: Test on riscv64 QEMU — verify console output
- [ ] Step 13: Test on aarch64 QEMU — verify console output
- [ ] Step 14: Commit

**Expected QEMU Output (riscv64):**
```
U-Boot 20XX.XX (...)
...
## Loading kernel from FIT Image ...
[0][0 0 INFO ] FDT: found 25 nodes, 4 CPUs
[1][0 0 INFO ] Memory: 1024 MB
[2][0 0 INFO ] Hello SimpleKernel
```

**验证命令：**
```bash
cargo build --target targets/riscv64gc-simplekernel.json
cargo xtask run --arch riscv64
# 观察串口输出，确认 "Hello SimpleKernel" 出现
# Ctrl+A, X 退出
```

**Exit Criteria:**
1. "Hello SimpleKernel" 出现在两个架构的 QEMU 串口输出中
2. 日志格式为 `[seq][core_id printer_core LEVEL]`
3. FDT 解析成功（显示节点数和 CPU 数）
4. Per-CPU 数据初始化完成

**你可以修改的内容：**
- `logging.rs` — 日志格式、级别过滤、队列大小
- `console.rs` — 串口驱动实现
- `fdt.rs` — FDT 解析逻辑、节点遍历方式
- `init.rs` — 启动流程、初始化顺序
- `error.rs` — 错误码定义、错误消息

---

### Phase 2: Core Infrastructure

**Goal:** Synchronization primitives, ELF parser, panic handler, IO buffer — all shared infrastructure needed by later phases.

**Depends on:** Phase 1

**Files to create:**
- `kernel/src/sync/spinlock.rs` — Custom SpinLock with interrupt save/restore
- `kernel/src/sync/mod.rs`
- `kernel/src/elf.rs` — ELF symbol table parser
- `kernel/src/panic.rs` — Panic handler with backtrace + observer pattern
- `kernel/src/io_buffer.rs` — RAII aligned I/O buffers

**C++ reference files:**
- `src/include/spinlock.hpp` → `kernel/src/sync/spinlock.rs`
- `src/include/kernel_elf.hpp` → `kernel/src/elf.rs`
- `src/include/panic_observer.hpp` → `kernel/src/panic.rs`
- `src/include/io_buffer.hpp` → `kernel/src/io_buffer.rs`

**Steps:**
- [ ] Step 1: Implement `SpinLock` with interrupt disable/enable on lock/unlock
- [ ] Step 2: Write unit tests for SpinLock (lock, unlock, guard drop)
- [ ] Step 3: Implement ELF parser (parse symbol table, string table)
- [ ] Step 4: Write unit tests for ELF parser (use test binary data)
- [ ] Step 5: Implement panic handler with DumpStack support
- [ ] Step 6: Implement IO buffer (aligned allocation wrapper)
- [ ] Step 7: **TCB/Scheduler ownership prototype (MANDATORY GATE)**

    Before Phase 5 begins, validate the TCB ownership model. Create a minimal prototype:
    ```rust
    // Prove that TaskControlBlock can be:
    // 1. Stored in a global task table (BTreeMap<Pid, Box<TaskControlBlock>>)
    // 2. Referenced by scheduler run queues (raw pointers or Pin<&mut TCB>)
    // 3. Referenced by per-CPU "current_task" pointer
    // 4. Safely accessed under SpinLock protection
    //
    // The prototype must demonstrate:
    // - Enqueue/dequeue a TCB into a mock scheduler
    // - Switch "current_task" pointer between two TCBs
    // - Access TCB fields through both the task table and the scheduler
    //
    // Decision: Document whether to use:
    // (a) UnsafeCell + raw pointers with SpinLock-guarded access (likely)
    // (b) Arc<SpinLock<TCB>> (too much overhead for scheduler hot path)
    // (c) Intrusive linked list (most C-like, least Rust-idiomatic)
    //
    // This prototype MUST pass before Phase 5 proceeds.
    ```

- [ ] Step 8: Commit

**Expected QEMU Output (riscv64):**
```
[0][0 0 INFO ] Hello SimpleKernel
[1][0 0 INFO ] Testing SpinLock... OK
[2][0 0 INFO ] Testing ELF parser... found 42 symbols
[3][0 0 INFO ] Testing panic handler...
PANIC at kernel/src/main.rs:XX: test panic
  backtrace:
    #0: 0x80200XXX - main::test_panic
    #1: 0x80200XXX - _start
[4][0 0 INFO ] TCB ownership prototype... OK
[5][0 0 INFO ] Phase 2 complete
```

**验证命令：**
```bash
# 单元测试（宿主机）
cargo test --lib
# QEMU 运行
cargo xtask run --arch riscv64
```

**Exit Criteria:**
1. `cargo test --lib` 全部通过（SpinLock、ELF parser 测试）
2. QEMU 中 panic 显示调用栈（函数名从 ELF 符号表解析）
3. SpinLock 在 QEMU 中正确禁用/恢复中断
4. **TCB 所有权原型编译通过并测试通过 — 所有权模型决策已记录**

**你可以修改的内容：**
- `spinlock.rs` — 锁策略、中断保存方式、锁级别层次
- `elf.rs` — ELF 解析实现
- `panic.rs` — panic 格式、调用栈输出格式
- `io_buffer.rs` — 缓冲区对齐方式
- TCB 所有权模型 — 可以选择不同的设计方案

---

### Phase 3: Memory Management

**Goal:** Physical frame allocator + virtual memory (page table) for both architectures.

**Depends on:** Phase 2

**Files to create:**
- `kernel/src/memory/mod.rs`
- `kernel/src/memory/address.rs` — `PhysAddr`, `VirtAddr` types with conversion
- `kernel/src/memory/physical_memory.rs` — Frame allocator (wraps buddy allocator)
- `kernel/src/memory/virtual_memory.rs` — Page table operations
- `kernel/src/memory/heap.rs` — `#[global_allocator]` implementation

**C++ reference files:**
- `src/memory/include/virtual_memory.hpp` → `kernel/src/memory/virtual_memory.rs`
- `src/memory/virtual_memory.cpp` → `kernel/src/memory/virtual_memory.rs`
- `src/memory/memory.cpp` → `kernel/src/memory/mod.rs` (MemoryInit)

**Steps:**
- [ ] Step 1: Implement `address.rs` — PhysAddr/VirtAddr newtypes with arithmetic
- [ ] Step 2: Implement `heap.rs` — GlobalAlloc wrapping `buddy_system_allocator`
- [ ] Step 3: Implement `physical_memory.rs` — frame allocator
- [ ] Step 4: Implement `virtual_memory.rs` — MapPage, UnmapPage, GetMapping
- [ ] Step 5: Implement ClonePageDirectory, DestroyPageDirectory
- [ ] Step 6: Implement MapMMIO for device memory
- [ ] Step 7: Implement `MemoryInit()` — setup kernel page table, enable paging
- [ ] Step 8: Implement `MemoryInitSMP()` for secondary cores
- [ ] Step 9: Write unit tests for address types
- [ ] Step 10: Write unit tests for virtual memory operations
- [ ] Step 11: Test in QEMU — verify paging works, kernel continues to run
- [ ] Step 12: Commit

**Expected QEMU Output:**
```
[0][0 0 INFO ] Hello SimpleKernel
[1][0 0 INFO ] MemoryInit: setting up page tables...
[2][0 0 INFO ] MemoryInit: mapping kernel 0x80200000-0x80400000
[3][0 0 INFO ] MemoryInit: paging enabled
[4][0 0 INFO ] HeapInit: 4MB heap at 0x80500000
[5][0 0 INFO ] HeapTest: Box::new(42) = 42 ✓
[6][0 0 INFO ] Phase 3 complete — paging active, heap available
```

**验证命令：**
```bash
cargo test --lib  # 地址类型、页表操作测试
cargo xtask run --arch riscv64  # 观察分页启用后内核继续运行
```

**Exit Criteria:**
1. 启用分页后内核继续运行（不 page fault 崩溃）
2. `Box::new()` 在堆初始化后成功（global allocator 工作）
3. `MapPage`/`UnmapPage` 单元测试通过
4. `ClonePageDirectory` 生成独立副本
5. SMP 核心分页初始化成功

**你可以修改的内容：**
- `virtual_memory.rs` — 页表操作实现、映射策略
- `physical_memory.rs` — 帧分配器算法
- `heap.rs` — 堆大小、分配器选择
- `address.rs` — 地址类型设计

---

### Phase 4: Interrupt & Timer

**Goal:** Interrupt handling (PLIC/GIC), timer tick, syscall dispatch.

**Depends on:** Phase 3

**Files to create:**
- `kernel/src/arch/riscv64/interrupt.rs` — PLIC + trap dispatch
- `kernel/src/arch/riscv64/timer.rs` — SBI timer
- `kernel/src/arch/riscv64/syscall.rs` — ecall handling
- `kernel/src/arch/riscv64/context.rs` — TrapContext, InitTaskContext
- `kernel/src/arch/riscv64/backtrace.rs`
- `kernel/src/arch/aarch64/interrupt.rs` — GICv3 + exception dispatch
- `kernel/src/arch/aarch64/timer.rs` — Generic Timer
- `kernel/src/arch/aarch64/syscall.rs` — svc handling
- `kernel/src/arch/aarch64/context.rs` — TrapContext, InitTaskContext
- `kernel/src/arch/aarch64/backtrace.rs`
- `kernel/src/syscall.rs` — Syscall number definitions + dispatch table

**Assembly files to reuse:**
- `interrupt.S` (both archs) — trap vector entry
- `switch.S` (both archs) — context save/restore

**C++ reference files:**
- `src/arch/riscv64/interrupt.cpp`, `interrupt_main.cpp`, `plic/plic.cpp`
- `src/arch/aarch64/interrupt.cpp`, `interrupt_main.cpp`, `gic/gic.cpp`
- `src/arch/*/timer.cpp`, `syscall.cpp`, `backtrace.cpp`
- `src/include/interrupt_base.h` → `Interrupt` trait
- `src/include/syscall.hpp`
- `src/include/tick_observer.hpp`

**Steps:**
- [ ] Step 1: Define `Interrupt` trait (replaces `InterruptBase` ABC)
- [ ] Step 2: Define `TickObserver` trait
- [ ] Step 3: Implement riscv64 interrupt (PLIC driver + dispatch)
- [ ] Step 4: Implement aarch64 interrupt (GICv3 driver + dispatch)
- [ ] Step 5: Implement riscv64 timer (SBI timer init + tick handler)
- [ ] Step 6: Implement aarch64 timer (Generic Timer init + tick handler)
- [ ] Step 7: Implement TrapContext / InitTaskContext for both archs
- [ ] Step 8: Implement syscall dispatch (syscall number → handler routing)
- [ ] Step 9: Implement backtrace for both archs
- [ ] Step 10: Wire InterruptInit/TimerInit into boot sequence
- [ ] Step 11: Copy and integrate interrupt.S / switch.S for both archs
- [ ] Step 12: Test in QEMU — verify timer interrupts fire, console still works
- [ ] Step 13: Commit

**Expected QEMU Output:**
```
[0][0 0 INFO ] Hello SimpleKernel
...
[N][0 0 INFO ] InterruptInit: PLIC configured, 32 IRQs
[N][0 0 INFO ] TimerInit: tick frequency = 1000 Hz
[N][0 0 INFO ] Tick #1
[N][0 0 INFO ] Tick #2
[N][0 0 INFO ] Tick #3
... (定时器中断持续触发)
```

**验证命令：**
```bash
cargo xtask run --arch riscv64
# 观察 "Tick #N" 持续递增 — 定时器中断在工作
# Ctrl+A, X 退出
```

**Exit Criteria:**
1. 定时器 tick 日志持续出现（~1000 Hz）
2. PLIC/GIC 外部中断正确分发
3. SpinLock 周围中断禁用/启用正确
4. `switch.S` ABI: `sizeof(CalleeSavedContext)` 匹配 Rust `#[repr(C)]` 结构体
5. SMP: 从核定时器中断在 `TimerInitSMP()` 后触发
6. 调用栈显示正确的函数名

**你可以修改的内容：**
- `interrupt.rs` — 中断控制器配置、分发逻辑
- `timer.rs` — 定时器频率、tick 处理逻辑
- `syscall.rs` — 系统调用号定义、分发表
- `context.rs` — TrapContext 结构体布局

---

### Phase 5: Task Management & Scheduling

**Goal:** Full task subsystem — TCB, schedulers (CFS/FIFO/RR), clone/exit/wait/sleep/signal, SMP load balancing.

**Depends on:** Phase 4

**Files to create:**
- `kernel/src/task/mod.rs`
- `kernel/src/task/tcb.rs` — TaskControlBlock
- `kernel/src/task/fsm.rs` — Task state machine
- `kernel/src/task/manager.rs` — TaskManager (global singleton)
- `kernel/src/task/scheduler/mod.rs` — `Scheduler` trait
- `kernel/src/task/scheduler/cfs.rs`
- `kernel/src/task/scheduler/fifo.rs`
- `kernel/src/task/scheduler/round_robin.rs`
- `kernel/src/task/clone.rs`
- `kernel/src/task/exit.rs`
- `kernel/src/task/wait.rs`
- `kernel/src/task/sleep.rs`
- `kernel/src/task/block.rs`
- `kernel/src/task/wakeup.rs`
- `kernel/src/task/balance.rs`
- `kernel/src/task/signal.rs`
- `kernel/src/task/mutex.rs` (blocking mutex)
- `kernel/src/task/messages.rs` — Task FSM messages (9 types: Schedule, Yield, Sleep, Block, Wakeup, Exit, Reap, Stop, Cont) + lifecycle messages (ThreadCreate, ThreadExit) as Rust enum
- `kernel/src/task/resource_id.rs` — ResourceType enum (10 types) + ResourceId (8-bit type + 56-bit data, Hash impl)
- `kernel/src/sync/mutex.rs` (updated)

**C++ reference files:**
- `src/task/include/*.hpp` (all 10 headers)
- `src/task/*.cpp` (all 12 implementation files)
- `src/include/mutex.hpp` → `kernel/src/task/mutex.rs`
- `src/include/signal.hpp` → `kernel/src/task/signal.rs`
- `src/task/include/task_messages.hpp` → `kernel/src/task/messages.rs`
- `src/task/include/lifecycle_messages.hpp` → `kernel/src/task/messages.rs`
- `src/task/include/resource_id.hpp` → `kernel/src/task/resource_id.rs`

**Steps:**
- [ ] Step 1: Define `Scheduler` trait (port from `SchedulerBase`)

```rust
/// Complete scheduler trait — ALL hooks from C++ SchedulerBase must be present.
pub trait Scheduler: Send + Sync {
    /// Add task to the ready queue.
    fn enqueue(&mut self, task: &mut TaskControlBlock);
    /// Remove task from the ready queue (for blocking/exit).
    fn dequeue(&mut self, task: &mut TaskControlBlock);
    /// Select the next task to run (does NOT remove from queue).
    fn pick_next(&mut self) -> Option<&mut TaskControlBlock>;
    /// Number of tasks in the ready queue.
    fn queue_size(&self) -> usize;
    /// Whether the queue is empty.
    fn is_empty(&self) -> bool;
    /// Tick update — returns true if preemption needed.
    fn on_tick(&mut self, current: &mut TaskControlBlock) -> bool { false }
    /// Time slice expired — returns true if task should be re-enqueued.
    fn on_time_slice_expired(&mut self, task: &mut TaskControlBlock) -> bool { true }
    /// Priority inheritance: boost task priority when higher-priority task waits on it.
    fn boost_priority(&mut self, _task: &mut TaskControlBlock, _new_priority: i32) {}
    /// Restore original priority after resource release.
    fn restore_priority(&mut self, _task: &mut TaskControlBlock) {}
    /// Called when task is preempted (Running → Ready).
    fn on_preempted(&mut self, _task: &mut TaskControlBlock) {}
    /// Called when task starts running (Ready → Running).
    fn on_scheduled(&mut self, _task: &mut TaskControlBlock) {}
    /// Get scheduler statistics.
    fn stats(&self) -> &SchedulerStats;
    /// Reset statistics.
    fn reset_stats(&mut self);
}

/// Scheduler statistics (matches C++ SchedulerBase::Stats)
pub struct SchedulerStats {
    pub total_enqueues: usize,
    pub total_dequeues: usize,
    pub total_picks: usize,
    pub total_preemptions: usize,
}
```

- [ ] Step 2: Implement ResourceId and ResourceType (port from `resource_id.hpp`)
- [ ] Step 3: Implement task messages as Rust enum (port from `task_messages.hpp` + `lifecycle_messages.hpp`)
- [ ] Step 4: Implement TaskControlBlock (port from `task_control_block.hpp`)
- [ ] Step 3: Implement Task FSM (state machine with valid transitions)
- [ ] Step 4: Implement FIFO scheduler + unit tests
- [ ] Step 5: Implement Round-Robin scheduler + unit tests
- [ ] Step 6: Implement CFS scheduler + unit tests
- [ ] Step 7: Implement TaskManager core (AddTask, Schedule, GetCurrentTask)
- [ ] Step 8: Implement clone (task creation)
- [ ] Step 9: Implement exit (task termination + zombie cleanup)
- [ ] Step 10: Implement wait (wait for child process)
- [ ] Step 11: Implement sleep (timed suspension)
- [ ] Step 12: Implement block/wakeup (resource-based blocking)
- [ ] Step 13: Implement signal subsystem
- [ ] Step 14: Implement blocking Mutex
- [ ] Step 15: Implement load balancing (cross-core work stealing)
- [ ] Step 16: Wire TaskManager into boot sequence (InitCurrentCore, Schedule)
- [ ] Step 17: Test in QEMU — verify task switching, idle task, timer preemption
- [ ] Step 18: Run scheduler unit tests
- [ ] Step 19: Commit

**Expected QEMU Output:**
```
[0][0 0 INFO ] Hello SimpleKernel
...
[N][0 0 INFO ] TaskManager: init process (pid 1) created
[N][0 0 INFO ] TaskManager: idle thread (pid 0) created
[N][0 0 INFO ] Task [pid=2] "test_thread" started
[N][0 0 INFO ] Task [pid=2] sleeping 100ms...
[N][0 0 INFO ] Task [pid=3] "worker" running, count=1
[N][0 0 INFO ] Task [pid=2] woke up
[N][0 0 INFO ] Task [pid=3] exiting with code 0
[N][0 0 INFO ] Task [pid=1] reaped zombie pid=3
[N][1 0 INFO ] SMP: core 1 online, idle thread running
[N][1 0 INFO ] Balance: stole task pid=4 from core 0
```

**验证命令：**
```bash
cargo test --lib  # CFS/FIFO/RR 调度器单元测试
cargo xtask run --arch riscv64  # 观察任务创建、切换、退出
```

**Exit Criteria:**
1. CFS/FIFO/RR 调度器单元测试通过
2. QEMU 中多个内核线程运行并切换
3. idle 线程让出 CPU 给就绪任务
4. Sleep/Wakeup/Block 正常工作
5. Clone 创建子任务，Exit 变成 zombie，Wait 回收
6. Signal 传递工作（SIGKILL 终止任务）
7. SMP: 跨核负载均衡（2 核配置下可观察）
8. 阻塞 Mutex: 竞争时任务正确阻塞和唤醒

**你可以修改的内容：**
- `scheduler/*.rs` — 调度算法实现
- `tcb.rs` — 任务控制块字段、状态机
- `clone.rs/exit.rs/wait.rs` — 任务生命周期逻辑
- `signal.rs` — 信号处理策略
- `balance.rs` — 负载均衡策略（窃取阈值等）
- `mutex.rs` — 阻塞互斥锁实现

---

### Phase 6: Device Framework & Drivers

**Goal:** Device manager, driver registry, platform bus (FDT enumeration), UART drivers, VirtIO subsystem.

**Depends on:** Phase 5

**Can parallelize with Phase 7.**

**Files to create:**
- `kernel/src/device/mod.rs`
- `kernel/src/device/manager.rs` — DeviceManager
- `kernel/src/device/registry.rs` — DriverRegistry
- `kernel/src/device/platform_bus.rs` — FDT-based device enumeration
- `kernel/src/device/node.rs` — DeviceNode
- `kernel/src/device/ns16550a.rs` — NS16550A UART
- `kernel/src/device/pl011.rs` — PL011 UART
- `kernel/src/device/virtio/mod.rs`
- `kernel/src/device/virtio/transport.rs` — VirtIO MMIO transport
- `kernel/src/device/virtio/queue.rs` — Split virtqueue
- `kernel/src/device/virtio/blk.rs` — VirtIO block device
- `kernel/src/device/virtio/console.rs`
- `kernel/src/device/virtio/gpu.rs`
- `kernel/src/device/virtio/input.rs`
- `kernel/src/device/virtio/net.rs`

**C++ reference files:**
- `src/device/include/*.hpp` (4 headers)
- `src/device/*.cpp` (2 implementation files)
- `src/device/ns16550a/*.hpp`
- `src/device/pl011/*.hpp`
- `src/device/virtio/**/*` (all VirtIO files)

**Steps:**
- [ ] Step 1: Define Driver trait (Probe/Remove pattern)
- [ ] Step 2: Implement DeviceNode
- [ ] Step 3: Implement DriverRegistry
- [ ] Step 4: Implement PlatformBus (FDT walk → driver matching)
- [ ] Step 5: Implement DeviceManager
- [ ] Step 6: Implement NS16550A driver
- [ ] Step 7: Implement PL011 driver
- [ ] Step 8: Implement VirtIO MMIO transport layer
- [ ] Step 9: Implement split virtqueue
- [ ] Step 10: Implement VirtIO block device driver
- [ ] Step 11: Implement VirtIO console driver (stub)
- [ ] Step 12: Implement VirtIO GPU driver (stub)
- [ ] Step 13: Implement VirtIO input driver (stub)
- [ ] Step 14: Implement VirtIO network driver (stub)
- [ ] Step 15: Wire DeviceInit into boot sequence
- [ ] Step 16: Test in QEMU — verify UART works via device framework
- [ ] Step 17: Test VirtIO block device read/write
- [ ] Step 18: Commit

- [ ] Step 19: Implement ACPI table parser (RSDP, XSDT, FADT structures — port from `acpi.hpp`)
- [ ] Step 20: Commit

**Exit Criteria:**
1. Devices enumerated from FDT — log shows discovered devices
2. UART output works via device framework (not just early console)
3. VirtIO block device read/write succeeds (verified with rootfs.img)
4. Driver registry matches `compatible` strings from FDT correctly
5. DeviceInit() 在两个架构上无错误完成
6. ACPI 表结构解析正确

**Expected QEMU Output:**
```
[N][0 0 INFO ] DeviceInit: scanning FDT...
[N][0 0 INFO ] PlatformBus: found "ns16550a" at 0x10000000
[N][0 0 INFO ] PlatformBus: found "virtio,mmio" at 0x10001000
[N][0 0 INFO ] NS16550A: probed, UART ready
[N][0 0 INFO ] VirtIO: block device, capacity=64MB
[N][0 0 INFO ] DeviceInit complete: 5 devices enumerated
[N][0 0 INFO ] (此后日志通过设备框架的 UART 驱动输出)
```

**你可以修改的内容：**
- `registry.rs` — 驱动匹配逻辑
- `platform_bus.rs` — FDT 遍历和设备枚举
- `ns16550a.rs/pl011.rs` — UART 驱动实现
- `virtio/*.rs` — VirtIO 协议实现
- `acpi.rs` — ACPI 表解析

---

### Phase 7: Filesystem

**Goal:** VFS layer, RamFS, FatFS, file descriptors.

**Depends on:** Phase 5 (for TaskManager). Phase 6 partially (FatFS needs VirtIO block device).

**Parallelization note:** VFS core + RamFS (Steps 1-5) can start as soon as Phase 5 is done, in parallel with Phase 6. FatFS (Step 6) must wait for Phase 6's VirtIO block device to be ready. Split execution accordingly.

**Files to create:**
- `kernel/src/fs/mod.rs`
- `kernel/src/fs/vfs.rs` — VFS operations
- `kernel/src/fs/file_descriptor.rs` — FD table
- `kernel/src/fs/ramfs.rs` — RAM filesystem
- `kernel/src/fs/fatfs.rs` — FAT filesystem (wraps VirtIO block device)

**C++ reference files:**
- `src/filesystem/include/file_descriptor.hpp`
- `src/filesystem/vfs/*.cpp` (15 files!)
- `src/filesystem/vfs/include/*`
- `src/filesystem/ramfs/*`
- `src/filesystem/fatfs/*`

**Steps:**
- [ ] Step 1: Define VFS traits (FileSystem, Inode, File)
- [ ] Step 2: Implement file descriptor table
- [ ] Step 3: Implement VFS layer (open, close, read, write, seek, mkdir, rmdir, unlink, readdir, mount, lookup)
- [ ] Step 4: Implement RamFS
- [ ] Step 5: Write unit tests for RamFS + VFS
- [ ] Step 6: Implement FatFS adapter (wrapping VirtIO block device)
- [ ] Step 7: Wire FileSystemInit into boot sequence
- [ ] Step 8: Test in QEMU — verify file operations
- [ ] Step 9: Commit

**Expected QEMU Output:**
```
[N][0 0 INFO ] FileSystemInit: mounting RamFS at /
[N][0 0 INFO ] VFS: / mounted (ramfs)
[N][0 0 INFO ] VFS test: mkdir /tmp OK
[N][0 0 INFO ] VFS test: create /tmp/hello.txt OK
[N][0 0 INFO ] VFS test: write 13 bytes OK
[N][0 0 INFO ] VFS test: read "Hello, world!" OK
[N][0 0 INFO ] VFS test: unlink /tmp/hello.txt OK
[N][0 0 INFO ] FatFS: mounting VirtIO block device at /mnt
[N][0 0 INFO ] FatFS: found 0 files in root directory
[N][0 0 INFO ] FileSystemInit complete
[N][0 0 INFO ] === SimpleKernel fully booted ===
```

**验证命令：**
```bash
cargo test --lib  # VFS、RamFS 单元测试
cargo xtask run --arch riscv64  # 完整启动序列
```

**Exit Criteria:**
1. RamFS: create/read/write/delete/mkdir/rmdir/readdir 全部工作
2. VFS: 挂载 RamFS 到 `/`，路径查找正确
3. 文件描述符: open/close/read/write/seek 通过 FD 表工作
4. FatFS: 从 VirtIO 块设备挂载
5. FileSystemInit() 无错误完成
6. 完整启动序列在 QEMU 中运行到 "fully booted"

**你可以修改的内容：**
- `vfs.rs` — VFS 层接口设计
- `file_descriptor.rs` — FD 表实现
- `ramfs.rs` — RamFS 实现
- `fatfs.rs` — FatFS 适配层

---

## 7. Testing Strategy

### 7.1 Test Layers

| Layer | Tool | Scope | Where |
|-------|------|-------|-------|
| **Unit tests** | `#[cfg(test)]` + `cargo test` | Individual functions, data structures | `kernel/src/**/*.rs` (in-file `#[cfg(test)]` modules) |
| **Integration tests** | Custom `no_std` test harness | Cross-module interactions | `kernel/tests/` |
| **System tests** | QEMU + serial output parsing | Full boot, device I/O, scheduling | `tests/system_test/` |
| **Architecture tests** | QEMU per-arch | Arch-specific correctness | Part of system tests |

### 7.2 Unit Test Pattern (Host-Only)

```rust
// In kernel/src/task/scheduler/cfs.rs
#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_cfs_enqueue_dequeue() {
        let mut scheduler = CfsScheduler::new();
        let mut task = TaskControlBlock::new_test(1, SchedPolicy::Cfs);
        scheduler.enqueue(&mut task);
        assert_eq!(scheduler.queue_size(), 1);
        let next = scheduler.pick_next();
        assert!(next.is_some());
    }

    #[test]
    fn test_cfs_vruntime_ordering() {
        // Tasks with lower vruntime are picked first
        // ...
    }
}
```

### 7.3 QEMU System Test Pattern

```rust
// Custom test runner for no_std QEMU tests
#![no_std]
#![no_main]
#![feature(custom_test_frameworks)]
#![test_runner(test_runner)]
#![reexport_test_harness_main = "test_main"]

fn test_runner(tests: &[&dyn Fn()]) {
    serial_println!("Running {} tests", tests.len());
    for test in tests {
        test();
    }
    // Exit QEMU with success code
    exit_qemu(QemuExitCode::Success);
}
```

### 7.4 Test Coverage Target

Maintain test parity with existing C++ tests:

| C++ Test File | Rust Equivalent |
|---------------|-----------------|
| `spinlock_test.cpp` | `sync/spinlock.rs` `#[cfg(test)]` |
| `kernel_fdt_test.cpp` | `fdt.rs` `#[cfg(test)]` |
| `kernel_elf_test.cpp` | `elf.rs` `#[cfg(test)]` |
| `cfs_scheduler_test.cpp` | `task/scheduler/cfs.rs` `#[cfg(test)]` |
| `fifo_scheduler_test.cpp` | `task/scheduler/fifo.rs` `#[cfg(test)]` |
| `rr_scheduler_test.cpp` | `task/scheduler/round_robin.rs` `#[cfg(test)]` |
| `virtual_memory_test.cpp` | `memory/virtual_memory.rs` `#[cfg(test)]` |
| `vfs_test.cpp` | `fs/vfs.rs` `#[cfg(test)]` |
| `ramfs_test.cpp` | `fs/ramfs.rs` `#[cfg(test)]` |
| `virtio_driver_test.cpp` | `device/virtio/mod.rs` `#[cfg(test)]` |
| `sk_string_test.cpp` | Not needed (Rust `core::str`) |
| `sk_libc_test.cpp` | Not needed (Rust stdlib) |
| `sk_ctype_test.cpp` | Not needed (Rust `core::char`) |
| `balance_test.cpp` | `task/balance.rs` `#[cfg(test)]` |
| `task/*.cpp` tests | `task/` module `#[cfg(test)]` |

---

## 8. Risk Assessment & Mitigation

### 8.1 High Risk

| Risk | Impact | Mitigation |
|------|--------|------------|
| **Assembly ↔ Rust ABI mismatch** | Boot fails, context switch corrupts | Test ABI calling convention exhaustively; add `static_assert` equivalents for struct layout with `#[repr(C)]` |
| **`no_std` crate ecosystem gaps** | Missing functionality, stuck on deps | Evaluate each crate upfront; have fallback plan to write from scratch |
| **Unsafe code correctness** | Memory safety bugs in `unsafe` blocks | Minimize `unsafe` surface; document every `unsafe` block; use `miri` where possible |
| **Linker script compatibility** | Rust output doesn't match expected sections | Test with `objdump` at each phase; may need section name adjustments |

| **Firmware boot-chain mismatch** | Kernel doesn't boot | The QEMU runner must replicate the full U-Boot FIT boot chain (not simple `-bios none`). Test boot early in Phase 0 with a minimal `.its` template. |
| **SMP bring-up handoff** | Secondary cores don't start | SBI `hart_start` (riscv64) and PSCI (aarch64) require correct entry point address and stack setup. Port and test with 2+ cores before Phase 5. |
| **Rust ownership vs scheduler structures** | Borrow checker fights with TCB management | TaskControlBlock is accessed from multiple contexts (scheduler queues, task table, per-CPU). Use `UnsafeCell` + raw pointers for scheduler internals (document safety invariants). Consider intrusive linked lists. |
| **Host vs target test divergence** | Unit tests pass on host but kernel crashes | Some modules (SpinLock, memory) behave differently in freestanding. Add QEMU smoke tests per phase, not just `cargo test`. |

### 8.2 Medium Risk

| Risk | Impact | Mitigation |
|------|--------|------------|
| **Nightly Rust instability** | Features break between versions | Pin nightly version in `rust-toolchain.toml`; test version upgrades separately |
| **Global allocator ordering** | Allocation before heap is initialized | Use `static` allocations for early boot; delay `#[global_allocator]` activation |
| **Rust binary size** | Larger than C++ binary | Use `opt-level`, LTO, `panic = "abort"`, strip dead code |
| **Build time** | Cargo slower than CMake for cross-compilation | Use `sccache`, incremental compilation, limit codegen-units |
| **FatFS C FFI complexity** | bindgen/libclang setup difficult for cross-compilation | Pre-generate bindings on host, check into repo; or write a minimal Rust-native FAT32 reader |

### 8.3 Low Risk

| Risk | Impact | Mitigation |
|------|--------|------------|
| **Firmware compatibility** | U-Boot/OpenSBI interface breaks | Firmware is unchanged; only kernel ABI matters |
| **QEMU version differences** | Test environment inconsistency | Pin QEMU version in dev container |
| **Dev Container toolchain** | Rust cross-compilation tools missing | Extend `.devcontainer/` to include rustup, nightly toolchain, cross-GCC, mkimage |

### 8.4 Key Decisions to Make Early

| Decision | Options | Recommendation |
|----------|---------|----------------|
| **Heap allocator** | `linked_list_allocator` vs `buddy_system_allocator` | `buddy_system_allocator` — better for page-granularity allocation |
| **Collections** | `heapless` (fixed-cap) vs `alloc` (heap) | Hybrid: `heapless` for hot paths (per-CPU scheduler queues), `alloc` for flexible structures |
| **Interrupt handling** | Trait objects (`dyn Interrupt`) vs enums | Enums for known set; trait objects for extensibility |
| **Singleton pattern** | `spin::Once<T>` vs `static mut` + `unsafe` | `spin::Once<T>` — safe, idiomatic |
| **Error handling** | Single `KernelError` vs per-module errors | Single `KernelError` with `ErrorCode` enum — matches C++ design |
| **CPU arch crates** | `riscv`/`aarch64-cpu` crates vs custom inline asm | Use crates when possible; custom asm for SimpleKernel-specific needs |

---

## 9. Preserving Interface-Driven Philosophy

The current C++ project's core philosophy is "interface-driven" — headers define contracts, AI generates implementations. This maps beautifully to Rust:

### 9.1 Rust Equivalent of Interface-Driven Design

| C++ Approach | Rust Approach |
|-------------|---------------|
| `.h` / `.hpp` header = interface contract | `trait` definition + type signatures = interface contract |
| Doxygen `@pre` / `@post` | Doc comments with `# Safety`, `# Panics`, `# Errors` sections |
| `.cpp` = AI-generated implementation | `impl` blocks = AI-generated implementation |
| GoogleTest = verification | `#[cfg(test)]` + `#[test]` = verification |

### 9.2 Example: Scheduler Interface (Rust)

```rust
/// Scheduler interface — abstract base for all scheduling policies.
///
/// Implement this trait to add a new scheduling algorithm.
/// The TaskManager dispatches to the appropriate scheduler based on
/// each task's `SchedPolicy`.
///
/// # Contract
/// - `enqueue` must be O(log n) or better
/// - `pick_next` returns the highest-priority ready task without removing it
/// - `on_tick` returns `true` if preemption is needed
///
/// # Known Implementations
/// - [`CfsScheduler`] — Completely Fair Scheduler (vruntime-based)
/// - [`FifoScheduler`] — First-In First-Out (no preemption)
/// - [`RoundRobinScheduler`] — Time-slice based preemption
pub trait Scheduler: Send + Sync {
    /// Add task to the ready queue.
    ///
    /// # Pre-conditions
    /// - `task.state` is `Ready`
    ///
    /// # Post-conditions
    /// - Task is in the scheduler's ready queue
    /// - `queue_size()` increased by 1
    fn enqueue(&mut self, task: &mut TaskControlBlock);

    /// Remove task from the ready queue (for blocking/exit).
    fn dequeue(&mut self, task: &mut TaskControlBlock);

    /// Select the next task to run (does NOT remove from queue).
    ///
    /// Returns `None` if the queue is empty.
    fn pick_next(&mut self) -> Option<&mut TaskControlBlock>;

    // ... etc
}
```

### 9.3 Learning Path Preserved

The Rust rewrite maintains the same learning flow:

```
1. Read trait definition → understand contract
2. Let AI generate `impl` block
3. Run tests to verify
4. Compare with reference implementation
```

---

## Appendix A: Command Reference (New vs Old)

| Old (CMake) | New (Cargo) |
|------------|-------------|
| `cmake --preset build_riscv64` | `cargo build --target targets/riscv64gc-simplekernel.json` |
| `cmake --preset build_aarch64` | `cargo build --target targets/aarch64-simplekernel.json` |
| `make SimpleKernel` | `cargo build --release` |
| `make run` | `cargo xtask run --arch riscv64` |
| `make debug` | `cargo xtask debug --arch riscv64` |
| `make unit-test` | `cargo test` (host) |
| `make coverage` | `cargo llvm-cov` |
| `pre-commit run --all-files` | `cargo fmt --check && cargo clippy` |

## Appendix B: Estimated Effort Breakdown

| Phase | Duration | Complexity | Parallelizable |
|-------|----------|------------|----------------|
| Phase 0: Skeleton | 1 week | Low | No (foundation) |
| Phase 1: Boot | 2 weeks | Medium | No (depends on P0) |
| Phase 2: Infrastructure | 1 week | Low-Medium | No (depends on P1) |
| Phase 3: Memory | 2 weeks | High | No (depends on P2) |
| Phase 4: Interrupts | 2 weeks | High | No (depends on P3) |
| Phase 5: Tasks | 3 weeks | Very High | No (depends on P4) |
| Phase 6: Devices | 3 weeks | Medium-High | Yes (with P7) |
| Phase 7: Filesystem | 2 weeks | Medium | Yes (with P6) |
| **Total** | **~16 weeks** | | |

## Appendix C: Nightly Features Required

```rust
#![feature(naked_functions)]         // For interrupt vector entry points
#![feature(alloc_error_handler)]     // Custom OOM handler
#![feature(custom_test_frameworks)]  // no_std testing
#![feature(panic_info_message)]      // Panic message extraction
#![feature(asm_const)]               // Constants in inline assembly
```

Note: Monitor stabilization progress. As features stabilize, remove `#![feature(...)]` flags.
