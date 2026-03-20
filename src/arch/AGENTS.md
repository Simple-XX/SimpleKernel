# AGENTS.md — src/arch/

## OVERVIEW
Per-architecture implementations of `arch.h` unified interface. Each arch has identical file structure: boot.S, arch_main.cpp, early_console.cpp, interrupt.S, interrupt_main.cpp, interrupt.cpp, timer.cpp, switch.S, syscall.cpp, backtrace.cpp, macro.S, link.ld.

## STRUCTURE
```
arch.h                  # Unified interface — ArchInit, InterruptInit, TimerInit, SyscallInit
aarch64/
  gic/                  # GICv3 interrupt controller (gic.cpp + include/gic.h)
  include/              # Arch-private headers
riscv64/
  plic/                 # PLIC interrupt controller (plic.cpp + include/plic.h)
  include/              # Arch-private headers
```

## WHERE TO LOOK
- **Boot entry** → `boot.S` → sets stack, jumps to `_start` in `src/main.cpp`
- **Arch init** → `arch_main.cpp` → `ArchInit()` configures FDT, console, SMP
- **Interrupt flow** → `interrupt.S` (trap vector) → `interrupt_main.cpp` (dispatch) → `interrupt.cpp` (controller)
- **Context switch** → `switch.S` → saves/restores registers, called by scheduler
- **Syscall path** → `syscall.cpp` → handles ecall/svc/int traps
- **Timer** → `timer.cpp` → arch-specific timer init and tick handler
- **Link layout** → `link.ld` → memory map, section placement, symbol exports

## CONVENTIONS
- Each arch directory mirrors the same file set — 1:1 correspondence
- Interrupt controllers live in subdirectories (gic/, plic/) with own CMakeLists.txt
- Assembly files use `.S` (preprocessed), `macro.S` for shared macros (riscv64)
- `include/` subdirs contain arch-private headers NOT exported to other modules
- `early_console.cpp` provides pre-device-framework output via arch-specific MMIO/SBI

## ANTI-PATTERNS
- **DO NOT** add arch-specific code outside `src/arch/{arch}/` — use `arch.h` abstraction
- **DO NOT** include arch-private headers from outside the arch directory
- `static_assert` guards on PerCpu alignment and GuardType ABI — do not remove
- `#error` in syscall.hpp for unsupported arch — intentional, add new arch case instead

## NOTES
- Boot chains: riscv64 (U-Boot SPL→OpenSBI→U-Boot), aarch64 (U-Boot→ATF→OP-TEE)
- SMP: aarch64=PSCI, riscv64=SBI hart_start
- Early console: aarch64=PL011 MMIO, riscv64=SBI call
- Backtrace uses ELF symbol table from `kernel_elf.hpp` for address→name resolution
