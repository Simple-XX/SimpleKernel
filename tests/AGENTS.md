# AGENTS.md — tests/

## OVERVIEW
Three test categories: unit_test (GoogleTest, host-only, ASan+coverage), system_test (freestanding, QEMU, custom runner), integration_test (minimal boot validation). Test binary: `tests/unit_test/unit-test`.

## STRUCTURE
```
unit_test/
  CMakeLists.txt            # gtest_discover_tests(), --coverage, -fsanitize=leak,address
  mocks/                    # Mock implementations for host testing
    arch.cpp                #   Stub arch functions
    cpu_io.h                #   Stub CPU I/O
    kernel.elf.h            #   Embedded ELF binary (~20k lines)
    dtb data files          #   Test device tree blobs
  task/                     # Task-specific test fixtures (TaskTestHarness)
  *_test.cpp                # Per-module: cfs_scheduler, fifo_scheduler, rr_scheduler,
                            #   kernel_elf, kernel_fdt, ramfs, vfs, virtual_memory,
                            #   spinlock, sk_libc, sk_ctype, kstd_vector, kstd_list, etc.
system_test/
  CMakeLists.txt            # Freestanding build, linked as kernel, runs in QEMU
  main.cpp                  # Custom test runner (NOT GoogleTest)
  system_test.h             # Test registration macros and runner framework
  *_test.cpp                # Per-module: schedulers, memory, interrupt, mutex,
                            #   spinlock, clone, exit, wait, fatfs, ramfs, etc.
integration_test/
  CMakeLists.txt            # Minimal boot tests
  aarch64_minimal/          # AArch64 bare-metal boot test
  opensbi_test/             # OpenSBI boot validation
```

## WHERE TO LOOK
- **Adding unit tests** → Create `tests/unit_test/{module}_test.cpp`, add to CMakeLists.txt
- **Adding system tests** → Create `tests/system_test/{module}_test.cpp`, register in main.cpp
- **Mock data** → `unit_test/mocks/` for stub implementations needed by host-only tests
- **Test fixtures** → `unit_test/task/` for `TaskTestHarness` scheduler test setup
- **Run unit tests** → `cd build_{arch} && make unit-test` (host architecture only)
- **Run system tests** → `cd build_{arch} && make run` (system tests embedded in kernel)

## CONVENTIONS
- Unit tests: GoogleTest, `gtest_discover_tests()`, host architecture only
- System tests: custom framework (`system_test.h`), runs inside QEMU as part of kernel
- One test file per module, named `{module}_test.cpp`
- libc/libcxx modules need BOTH unit_test AND system_test
- Arch/kernel code needs system_test only (can't run on host)
- Mocks stub out hardware-dependent code for host compilation
- `--coverage` + `-fsanitize=leak` + `-fsanitize=address` on unit tests

## ANTI-PATTERNS
- **DO NOT** use GoogleTest in system_test — it uses a custom freestanding runner
- **DO NOT** skip mocks for unit tests — host builds cannot access real hardware
- **DO NOT** forget to update CMakeLists.txt when adding test files
- **DO NOT** run unit tests on cross-compiled builds — host arch only
