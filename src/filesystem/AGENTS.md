# AGENTS.md — src/filesystem/

## OVERVIEW
Three-layer filesystem: VFS abstraction (types in vfs_types.hpp, filesystem ops in filesystem.hpp), RamFS (in-memory, static pool allocation), FatFS (3rd-party FAT wrapper over VirtIO block device). FileDescriptor table per-task.

## STRUCTURE
```
include/
  file_descriptor.hpp     # FileDescriptor — per-task FD table
filesystem.cpp            # FileSystemInit() — mounts root RamFS, registers FatFS
file_descriptor.cpp       # FD operations implementation
vfs/
  include/
    block_device.hpp      # Block device abstraction for filesystem I/O
    filesystem.hpp        # Filesystem type registration and superblock operations
    mount.hpp             # Mount point management
    vfs.hpp               # VFS public API — open, read, write, close, mkdir, etc.
    vfs_types.hpp         # Core VFS types: Inode, Dentry, File, Superblock, ops tables
  vfs_internal.hpp        # VFS internal helpers (not exported)
  vfs.cpp                 # VFS core — Dentry cache, superblock management
  open.cpp close.cpp read.cpp write.cpp  # Per-syscall VFS operations
  lookup.cpp mkdir.cpp rmdir.cpp unlink.cpp seek.cpp readdir.cpp mount.cpp
ramfs/
  include/
    ramfs.hpp             # RamFS — register_filesystem, Inode/Dentry ops
  ramfs.cpp               # RamFS implementation — static pool
fatfs/
  include/
    fatfs.hpp             # FatFS kernel wrapper
    ffconf.h              # FatFS configuration (3rd-party)
  fatfs.cpp               # FatFS → VFS bridge
  diskio.cpp              # FatFS disk I/O — routes to VirtIO block device
```

## WHERE TO LOOK
- **Adding a filesystem** → Implement ops from `vfs_types.hpp`, register via `filesystem.hpp`
- **VFS operations** → One .cpp per syscall in `vfs/` (open.cpp, read.cpp, etc.)
- **Block device I/O** → `fatfs/diskio.cpp` → `vfs/include/block_device.hpp`
- **Mount flow** → `filesystem.cpp` → mounts "/" as RamFS at boot

## CONVENTIONS
- VFS follows Linux-style layering: Superblock → Inode → Dentry → File (all types in `vfs_types.hpp`)
- One .cpp per VFS operation (same pattern as task/ syscalls)
- RamFS uses static pool allocation (no heap) — fixed-size inode/dentry arrays
- FatFS wraps 3rd-party `ff.h` library — `diskio.cpp` is the HAL adaptation layer
- `vfs_internal.hpp` is VFS-private — not included outside `vfs/`

## ANTI-PATTERNS
- **DO NOT** include `vfs_internal.hpp` outside `vfs/` directory
- **DO NOT** allocate heap in RamFS — uses static pools pre-sized at compile time
- **DO NOT** call filesystem operations before `FileSystemInit()` in boot sequence
- **DO NOT** bypass VFS layer to access RamFS/FatFS directly — use `vfs.hpp` API
