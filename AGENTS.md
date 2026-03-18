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

> Full reference: `docs/coding_standards.md` — read it before generating any code.
> Baseline example file: `src/include/spinlock.hpp`

- **C++ style**: see `docs/coding_standards.md` (naming, headers, includes, returns, attributes, Doxygen, error handling, freestanding constraints)
- **CMake**: UPPERCASE commands/keywords, 4-space indent, 80-char lines, space before `(`

## ANTI-PATTERNS

> See also `docs/coding_standards.md` §9–§10 for full prohibited-patterns list.

- **NO** modifying interface .h/.hpp files to add implementation
- **NO** `as any`/type suppression equivalents

## UNIQUE STYLES
- `etl::singleton<T>` with named aliases in `kernel.h` (e.g. `TaskManagerSingleton::instance()`, `DeviceManagerSingleton::create()`)
- `LockGuard<SpinLock>` RAII locking

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
