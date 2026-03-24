# Rust Kernel Migration: Quick Reference Guide

**For**: SimpleKernel C++23 → Rust rewrite
**Architectures**: riscv64, aarch64
**Timeline**: 6-9 months (phased approach)

---

## 1. ESSENTIAL CRATES (Copy-Paste Ready)

```toml
[package]
name = "simplekernel"
edition = "2024"
build = "build.rs"

[dependencies]
# Synchronization
spin = "0.9.8"                    # Spinlocks, Once, RwLock

# Memory
linked_list_allocator = "0.9.0"   # Heap allocator
arrayvec = "0.7.4"                # Stack-based Vec
smallvec = "1.15.1"               # Small vector optimization
slab = "0.4"                      # Slab allocator

# Bit manipulation
bitflags = "2"                    # Bit flags
bitfield = "0.13.2"               # Bit field structs

# Hardware access
volatile = "0.4"                  # Volatile reads/writes

# Binary parsing
object = { version = "0.37.1", default-features = false, features = ["read_core", "elf"] }

# Logging
log = "0.4"                       # Logging facade

# Architecture-specific
[target.'cfg(target_arch = "aarch64")'.dependencies]
fdt = { git = "https://github.com/repnop/fdt.git", rev = "2fb1409edd1877c714a0aa36b6a7c5351004be54" }

[target.'cfg(target_arch = "riscv64")'.dependencies]
fdt = { git = "https://github.com/repnop/fdt.git", rev = "2fb1409edd1877c714a0aa36b6a7c5351004be54" }
sbi-rt = "0.0.3"                  # RISC-V SBI runtime

[features]
default = ["serial_debug"]
serial_debug = []
multi_core = []

[profile.dev]
panic = "abort"

[profile.release]
panic = "abort"
debug = "full"
```

---

## 2. TARGET SPECIFICATIONS

### aarch64-unknown-kernel.json
```json
{
    "abi": "softfloat",
    "arch": "aarch64",
    "data-layout": "e-m:e-p270:32:32-p271:32:32-p272:64:64-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128-Fn32",
    "disable-redzone": true,
    "dynamic-linking": false,
    "executables": true,
    "features": "+strict-align,-neon,-fp-armv8,+tpidr-el1",
    "frame-pointer": "always",
    "linker": "rust-lld",
    "linker-flavor": "gnu-lld",
    "llvm-target": "aarch64-unknown-none",
    "no-default-libraries": true,
    "os": "none",
    "relocation-model": "pic",
    "target-pointer-width": "64"
}
```

### riscv64-unknown-kernel.json
```json
{
    "arch": "riscv64",
    "code-model": "medium",
    "data-layout": "e-m:e-p:64:64-i64:64-i128:128-n64-S128",
    "disable-redzone": true,
    "dynamic-linking": false,
    "executables": true,
    "features": "+m,+a,+f,+d,+c",
    "frame-pointer": "always",
    "linker": "rust-lld",
    "linker-flavor": "gnu-lld",
    "llvm-target": "riscv64-unknown-none-elf",
    "no-default-libraries": true,
    "os": "none",
    "relocation-model": "pic",
    "target-pointer-width": "64"
}
```

---

## 3. LINKER SCRIPT TEMPLATE (aarch64.ld)

```ld
ENTRY(_start)
OUTPUT_FORMAT(elf64-littleaarch64)

KERNEL_OFFSET = 0xFFFFFF8000000000;

SECTIONS {
    . = KERNEL_OFFSET;
    . += SIZEOF_HEADERS;

    .text ALIGN(4K) : AT(ADDR(.text) - KERNEL_OFFSET) {
        __text_start = .;
        *(.text*)
        __text_end = .;
    }

    .rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_OFFSET) {
        __rodata_start = .;
        *(.rodata*)
        __rodata_end = .;
    }

    .data ALIGN(4K) : AT(ADDR(.data) - KERNEL_OFFSET) {
        __data_start = .;
        *(.data*)
        . = ALIGN(4K);
        *(.bss*)
        __data_end = .;
    }

    __end = .;

    /DISCARD/ : {
        *(.comment*)
        *(.eh_frame*)
        *(.note*)
    }
}
```

---

## 4. BUILD.RS TEMPLATE

```rust
use std::{env, path::Path, process::Command};

fn main() {
    let arch = env::var("CARGO_CFG_TARGET_ARCH").unwrap();
    let out_dir = env::var("OUT_DIR").unwrap();

    println!("cargo:rerun-if-changed=build.rs");

    match &*arch {
        "aarch64" => {
            println!("cargo::rustc-cfg=dtb");
            // Assemble aarch64-specific code if needed
        }
        "riscv64" => {
            println!("cargo::rustc-cfg=dtb");
            // Assemble riscv64-specific code if needed
        }
        _ => {}
    }
}
```

---

## 5. KERNEL MAIN TEMPLATE

```rust
#![no_std]
#![no_main]
#![feature(asm_cfg)]

extern crate alloc;

use core::panic::PanicInfo;

mod arch;
mod memory;
mod interrupt;
mod task;
mod device;
mod filesystem;

#[no_mangle]
pub extern "C" fn kmain() -> ! {
    // Initialize architecture
    arch::init();

    // Initialize memory
    memory::init();

    // Initialize interrupts
    interrupt::init();

    // Initialize devices
    device::init();

    // Initialize filesystem
    filesystem::init();

    // Start scheduler
    task::schedule();
}

#[panic_handler]
fn panic(info: &PanicInfo) -> ! {
    // Print panic info
    loop {}
}
```

---

## 6. COMMON PATTERNS

### Spinlock Usage
```rust
use spin::Mutex;

static SCHEDULER: Mutex<Scheduler> = Mutex::new(Scheduler::new());

// Usage:
{
    let mut sched = SCHEDULER.lock();
    sched.enqueue(task);
} // Guard auto-released
```

### Lazy Initialization
```rust
use spin::Once;

static MEMORY_MANAGER: Once<MemoryManager> = Once::new();

// Initialize once:
MEMORY_MANAGER.call_once(|| MemoryManager::new());

// Use:
let mm = MEMORY_MANAGER.get().unwrap();
```

### Volatile MMIO Access
```rust
use volatile::Volatile;

struct UARTRegisters {
    data: Volatile<u8>,
    control: Volatile<u8>,
}

// Safe volatile access:
uart.data.write(byte);
let status = uart.control.read();
```

### Error Handling
```rust
fn allocate_page() -> Result<Page, AllocError> {
    let frame = ALLOCATOR.allocate()?;
    Ok(Page::from_frame(frame))
}
```

### Trait-Based Abstraction
```rust
pub trait InterruptController {
    fn enable(&mut self, irq: u32);
    fn disable(&mut self, irq: u32);
}

pub struct GIC { ... }
impl InterruptController for GIC { ... }
```

---

## 7. BUILD COMMANDS

```bash
# Build for aarch64
cargo build --target targets/aarch64-unknown-kernel.json

# Build for riscv64
cargo build --target targets/riscv64-unknown-kernel.json

# Run in QEMU (aarch64)
qemu-system-aarch64 -machine virt -cpu cortex-a72 \
    -kernel target/aarch64-unknown-kernel/debug/simplekernel

# Run in QEMU (riscv64)
qemu-system-riscv64 -machine virt -cpu rv64 \
    -kernel target/riscv64-unknown-kernel/debug/simplekernel

# Debug with GDB
qemu-system-aarch64 -machine virt -cpu cortex-a72 \
    -kernel target/aarch64-unknown-kernel/debug/simplekernel \
    -s -S &
gdb target/aarch64-unknown-kernel/debug/simplekernel
(gdb) target remote localhost:1234
```

---

## 8. CLIPPY LINTS FOR KERNEL

```toml
[lints.clippy]
# Prevent panics in kernel
unwrap_used = "warn"
expect_used = "warn"
panic = "deny"

# Prevent indexing panics
indexing_slicing = "warn"

# Prevent integer overflows
arithmetic_side_effects = "warn"

# Prevent unsafe pointer dereferencing
not_unsafe_ptr_arg_deref = "deny"
```

---

## 9. MIGRATION CHECKLIST

### Phase 1: Foundation (Weeks 1-4)
- [ ] Set up Cargo.toml with essential crates
- [ ] Create target specifications (aarch64, riscv64)
- [ ] Write linker scripts
- [ ] Port boot assembly (minimal changes)
- [ ] Implement basic arch abstraction (traits)
- [ ] Set up memory allocator

### Phase 2: Core Kernel (Weeks 5-12)
- [ ] Interrupt handling (trait-based)
- [ ] Task scheduling (Arc + Mutex)
- [ ] Synchronization primitives
- [ ] Virtual memory management
- [ ] Context switching

### Phase 3: Drivers (Weeks 13-20)
- [ ] Device framework (traits)
- [ ] UART driver
- [ ] Interrupt controller drivers (GIC, PLIC)
- [ ] Block device drivers

### Phase 4: Filesystem & Syscalls (Weeks 21-28)
- [ ] VFS abstraction
- [ ] RamFS implementation
- [ ] FatFS integration
- [ ] Syscall layer

### Phase 5: Testing (Weeks 29+)
- [ ] Unit tests
- [ ] Integration tests (QEMU)
- [ ] Performance profiling
- [ ] Documentation

---

## 10. REFERENCE PROJECTS

| Project | URL | Best For |
|---------|-----|----------|
| **rCore** | https://github.com/rcore-os/rCore | Learning, RISC-V |
| **Redox** | https://github.com/redox-os/kernel | Production patterns, microkernel |
| **Theseus** | https://github.com/apogeeoak/theseus | Type-safe design, intralingual patterns |
| **rCore-Tutorial** | https://rcore-os.github.io/rCore-Tutorial-Book-v3/ | Step-by-step guide |

---

## 11. COMMON GOTCHAS

1. **No panics in kernel**: Use `.get()` instead of `[]`, return `Result<T, E>`
2. **Volatile access**: Always use `volatile::Volatile` for MMIO
3. **Unsafe code**: Document with `// SAFETY:` comments
4. **Linker scripts**: Ensure `AT()` and `ADDR()` are correct for bootloader
5. **Target specs**: Disable SIMD (SSE, AVX) for kernel
6. **Allocator**: Initialize before using `Box`, `Vec`, `Arc`
7. **Interrupts**: Disable during critical sections (use spinlocks)

---

## 12. USEFUL COMMANDS

```bash
# Check for panics
cargo clippy --target targets/aarch64-unknown-kernel.json -- -D clippy::panic

# Generate documentation
cargo doc --target targets/aarch64-unknown-kernel.json --open

# Check code size
cargo build --target targets/aarch64-unknown-kernel.json --release
ls -lh target/aarch64-unknown-kernel/release/simplekernel

# Disassemble
llvm-objdump -d target/aarch64-unknown-kernel/release/simplekernel | head -100

# Check symbols
llvm-nm target/aarch64-unknown-kernel/release/simplekernel | grep -i "kernel\|main"
```

---

## NEXT STEPS

1. **Read**: RUST_KERNEL_ECOSYSTEM.md (full research document)
2. **Study**: rCore-Tutorial-v3 for RISC-V kernel basics
3. **Clone**: Redox kernel; analyze architecture
4. **Setup**: Create Cargo.toml, target specs, linker scripts
5. **Implement**: Start with Phase 1 (foundation)
6. **Test**: Boot in QEMU; iterate
