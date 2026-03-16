# AGENTS.md — SimpleKernel

## OVERVIEW
Interface-driven OS kernel for AI-assisted learning. C++23/C23, freestanding, no RTTI/exceptions. Three architectures: x86_64, riscv64, aarch64. Headers define contracts (Doxygen @pre/@post), AI generates .cpp implementations, tests verify compliance.

## STRUCTURE
```
src/include/          # Public interface headers — READ FIRST
src/arch/             # Per-architecture code (see src/arch/AGENTS.md)
src/device/           # Device framework, drivers (see src/device/AGENTS.md)
src/task/             # Schedulers, TCB, sync (see src/task/AGENTS.md)
src/filesystem/       # VFS, RamFS, FatFS (see src/filesystem/AGENTS.md)
src/memory/           # Virtual/physical memory management
src/libc/             # Kernel C stdlib (sk_ prefix headers)
src/libcxx/           # Kernel C++ runtime (kstd_ prefix headers)
tests/                # Unit/integration/system tests (see tests/AGENTS.md)
cmake/                # Toolchain files, build helpers
3rd/                  # Git submodules (opensbi, u-boot, googletest, bmalloc, ...)
```

## WHERE TO LOOK
- **Implementing a module** → Read interface header in `src/include/` first, then arch-specific in `src/arch/{arch}/`
- **Adding a driver** → `src/device/include/driver/` for examples, `driver_registry.hpp` for registration
- **Adding a scheduler** → `src/task/include/scheduler_base.hpp` for base class
- **Boot flow** → `src/main.cpp`: _start → ArchInit → MemoryInit → InterruptInit → DeviceInit → FileSystemInit → Schedule()
- **Error handling** → `Expected<T>` (std::expected alias) in `src/include/expected.hpp`
- **Logging** → `klog::Debug/Info/Warn/Err()` or `klog::info <<` stream API

## CODE MAP
| Interface | Purpose | Implementation |
|-----------|---------|----------------|
| `src/arch/arch.h` | Arch-agnostic entry points | `src/arch/{arch}/*.cpp` |
| `src/include/interrupt_base.h` | Interrupt subsystem ABC | `src/arch/{arch}/interrupt.cpp` |
| `src/include/kernel.h` | Singleton aliases, global includes | header-only (defines `TaskManagerSingleton`, `DeviceManagerSingleton`, etc.) |
| `src/memory/include/virtual_memory.hpp` | Virtual memory mgmt | `src/memory/virtual_memory.cpp` |
| `src/include/expected.hpp` | `Expected<T>`, `Error`, `ErrorCode` | header-only |
| `src/include/kernel_fdt.hpp` | Device tree parser | header-only (utility) |
| `src/include/kernel_elf.hpp` | ELF parser | header-only (utility) |
| `src/include/kernel_log.hpp` | Logging (MPMC queue, levels) | header-only |
| `src/include/spinlock.hpp` | Spinlock | header-only (__always_inline) |
| `src/include/mutex.hpp` | Mutex | `src/task/mutex.cpp` |
| `src/include/per_cpu.hpp` | Per-CPU data + singleton | header-only |
| `src/include/basic_info.hpp` | Kernel info (memory, cores) | header-only |
| `src/include/io_buffer.hpp` | RAII aligned I/O buffers | `src/io_buffer.cpp` |
| `src/include/syscall.hpp` | Syscall numbers + declarations | `src/syscall.cpp` |
| `src/include/mmio_accessor.hpp` | Generic MMIO register access | header-only |
| `src/include/panic_observer.hpp` | Panic event observer | header-only |
| `src/include/tick_observer.hpp` | Tick event observer | header-only |
| `src/include/kernel_config.hpp` | Task/observer limit constants | header-only |
| `src/device/include/*.hpp` | Device framework | header-only + `device.cpp`, `device_manager.cpp` |
| `src/task/include/*.hpp` | Task/scheduler interfaces | `src/task/*.cpp` |

## CONVENTIONS
- **Style**: Google C++ via `.clang-format`, enforced by pre-commit
- **Naming**: files=`snake_case`, classes=`PascalCase`, vars=`snake_case`, members=`snake_case_` (trailing _), constants=`kCamelCase`, macros=`SCREAMING_SNAKE`
- **Headers**: `#pragma once` guard (all headers use this consistently)
- **Copyright**: `/** @copyright Copyright The SimpleKernel Contributors */`
- **Includes**: system → third-party → project. Use `kstd_cstdio`/`kstd_vector`, never std dynamic alloc
- **Returns**: trailing return type `auto Func() -> RetType`
- **CMake**: UPPERCASE commands/keywords, 4-space indent, 80-char lines, space before `(`

## ANTI-PATTERNS
- **NO** exceptions, RTTI, `dynamic_cast`, `typeid`
- **NO** heap allocation before memory subsystem init — use `src/libc/` and `src/libcxx/`
- **NO** standard library dynamic containers in freestanding — use `sk_` prefixed versions
- **NO** implementation in interface headers (exception: `__always_inline` perf-critical, utility parsers)
- **NO** modifying interface .h/.hpp files to add implementation
- **NO** `as any`/type suppression equivalents

## UNIQUE STYLES
- `Expected<T>` for all error returns (no exceptions)
- `etl::singleton<T>` with named aliases in `kernel.h` (e.g. `TaskManagerSingleton::instance()`, `DeviceManagerSingleton::create()`)
- `LockGuard<SpinLock>` RAII locking
- `__builtin_unreachable()` for dead code paths
- `[[nodiscard]]`, `[[maybe_unused]]` attributes used extensively
- Doxygen `@pre`/`@post`/`@brief` on every interface method

## COMMANDS
```bash
git submodule update --init --recursive   # First clone setup
cmake --preset build_{riscv64|aarch64|x86_64}
cd build_{arch} && make SimpleKernel       # Build kernel (NOT 'make kernel')
make run                                   # Run in QEMU
make debug                                 # GDB on localhost:1234
cmake --preset build_{arch} && cd build_{arch} && make unit-test  # Host-only tests
make coverage                              # Tests + coverage report
pre-commit run --all-files                 # Format check
```

## NOTES
- Interface-driven: headers are contracts, .cpp files are implementations AI generates
- Boot chains differ: x86_64 (U-Boot), riscv64 (U-Boot SPL→OpenSBI→U-Boot), aarch64 (U-Boot→ATF→OP-TEE)
- aarch64 needs two serial terminal tasks (::54320, ::54321) before `make run`
- Unit tests only run on host arch (`build_{arch}` on {arch} host)
- Git commits: `<type>(<scope>): <subject>` with `--signoff`
- Debug artifacts in `build_{arch}/bin/` (objdump, nm, map, dts, QEMU logs)
