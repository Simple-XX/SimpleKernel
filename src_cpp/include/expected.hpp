/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cstdint>
#include <expected>

/// 内核错误码
enum class ErrorCode : uint64_t {
  kSuccess = 0,
  // ELF 相关错误 (0x100 - 0x1FF)
  kElfInvalidAddress = 0x100,
  kElfInvalidMagic = 0x101,
  kElfUnsupported32Bit = 0x102,
  kElfInvalidClass = 0x103,
  kElfSymtabNotFound = 0x104,
  kElfStrtabNotFound = 0x105,
  // FDT 相关错误 (0x200 - 0x2FF)
  kFdtInvalidAddress = 0x200,
  kFdtInvalidHeader = 0x201,
  kFdtNodeNotFound = 0x202,
  kFdtPropertyNotFound = 0x203,
  kFdtParseFailed = 0x204,
  kFdtInvalidPropertySize = 0x205,
  // SpinLock 相关错误 (0x300 - 0x3FF)
  kSpinLockRecursiveLock = 0x300,
  kSpinLockNotOwned = 0x301,
  // Mutex 相关错误 (0x380 - 0x3FF)
  kMutexNoTaskContext = 0x380,
  kMutexRecursiveLock = 0x381,
  kMutexNotOwned = 0x382,
  kMutexNotLocked = 0x383,
  // VirtualMemory 相关错误 (0x400 - 0x4FF)
  kVmAllocationFailed = 0x400,
  kVmMapFailed = 0x401,
  kVmUnmapFailed = 0x402,
  kVmInvalidPageTable = 0x403,
  kVmPageNotMapped = 0x404,
  // IPI 相关错误 (0x500 - 0x5FF)
  kIpiTargetOutOfRange = 0x500,
  kIpiSendFailed = 0x501,
  // Task 相关错误 (0x700 - 0x7FF)
  kTaskNoCurrentTask = 0x700,
  kTaskPidAllocationFailed = 0x701,
  kTaskAllocationFailed = 0x702,
  kTaskInvalidCloneFlags = 0x703,
  kTaskPageTableCloneFailed = 0x704,
  kTaskKernelStackAllocationFailed = 0x705,
  kTaskNoChildFound = 0x706,
  kTaskInvalidPid = 0x707,
  // Signal 相关错误 (0xC00 - 0xCFF)
  kSignalInvalidNumber = 0xC00,
  kSignalInvalidPid = 0xC01,
  kSignalPermissionDenied = 0xC02,
  kSignalUncatchable = 0xC03,
  kSignalTaskNotFound = 0xC04,
  // Device 相关错误 (0x800 - 0x8FF)
  kDeviceNotFound = 0x800,
  kDeviceAlreadyOpen = 0x801,
  kDeviceNotOpen = 0x802,
  kDeviceReadFailed = 0x803,
  kDeviceWriteFailed = 0x804,
  kDeviceIoctlFailed = 0x805,
  kDeviceMmapFailed = 0x806,
  kDeviceNotSupported = 0x807,
  kDeviceBusy = 0x808,
  kDevicePermissionDenied = 0x809,
  kDeviceInvalidOffset = 0x80A,
  kDeviceBlockUnaligned = 0x80B,
  kDeviceBlockOutOfRange = 0x80C,
  kDeviceFlushFailed = 0x80D,
  /// 通用设备报告错误
  kDeviceError = 0x80E,
  /// IO 操作错误
  kIoError = 0x80F,
  /// 不支持的操作（通用，非设备特定）
  kNotSupported = 0x810,
  /// 操作超时
  kTimeout = 0x811,
  // VirtIO 传输层错误 (0x820 - 0x82F)
  /// 无效的 MMIO 魔数
  kInvalidMagic = 0x820,
  /// 无效的版本号
  kInvalidVersion = 0x821,
  /// 无效的设备 ID（设备不存在）
  kInvalidDeviceId = 0x822,
  /// 传输层未正确初始化
  kTransportNotInitialized = 0x823,
  /// 特性协商失败
  kFeatureNegotiationFailed = 0x824,
  // VirtIO 虚拟队列错误 (0x830 - 0x83F)
  /// 队列不可用（queue_num_max == 0）
  kQueueNotAvailable = 0x830,
  /// 队列已被使用
  kQueueAlreadyUsed = 0x831,
  /// 请求的队列大小超过设备支持的最大值
  kQueueTooLarge = 0x832,
  /// 没有空闲描述符
  kNoFreeDescriptors = 0x833,
  /// 无效的描述符索引
  kInvalidDescriptor = 0x834,
  /// 没有已使用的缓冲区可回收
  kNoUsedBuffers = 0x835,
  // 文件系统相关错误 (0xA00 - 0xAFF)
  kFsFileNotFound = 0xA00,
  kFsPermissionDenied = 0xA01,
  kFsNotADirectory = 0xA02,
  kFsIsADirectory = 0xA03,
  kFsFileExists = 0xA04,
  kFsNoSpace = 0xA05,
  kFsMountFailed = 0xA06,
  kFsUnmountFailed = 0xA07,
  kFsInvalidPath = 0xA08,
  kFsFdTableFull = 0xA09,
  kFsInvalidFd = 0xA0A,
  kFsNotMounted = 0xA0B,
  kFsReadOnly = 0xA0C,
  kFsCorrupted = 0xA0D,
  kFsAlreadyMounted = 0xA0E,
  kFsNotEmpty = 0xA0F,
  // BlockDevice 相关错误 (0xB00 - 0xBFF)
  kBlkDeviceNotFound = 0xB00,
  kBlkReadFailed = 0xB01,
  kBlkWriteFailed = 0xB02,
  kBlkSectorOutOfRange = 0xB03,
  // IrqChip 相关错误 (0x900 - 0x9FF)
  kIrqChipInvalidIrq = 0x900,
  kIrqChipIrqNotEnabled = 0x901,
  kIrqChipAffinityFailed = 0x902,
  kIrqChipIpiTimeout = 0x903,
  // 通用错误 (0xF00 - 0xFFF)
  kInvalidArgument = 0xF00,
  kOutOfMemory = 0xF01,
};

/// 获取错误码对应的错误信息
constexpr auto GetErrorMessage(ErrorCode code) -> const char* {
  switch (code) {
    case ErrorCode::kSuccess:
      return "Success";
    case ErrorCode::kElfInvalidAddress:
      return "Invalid ELF address";
    case ErrorCode::kElfInvalidMagic:
      return "Invalid ELF magic number";
    case ErrorCode::kElfUnsupported32Bit:
      return "32-bit ELF not supported";
    case ErrorCode::kElfInvalidClass:
      return "Invalid ELF class";
    case ErrorCode::kElfSymtabNotFound:
      return ".symtab section not found";
    case ErrorCode::kElfStrtabNotFound:
      return ".strtab section not found";
    case ErrorCode::kFdtInvalidAddress:
      return "Invalid FDT address";
    case ErrorCode::kFdtInvalidHeader:
      return "Invalid FDT header";
    case ErrorCode::kFdtNodeNotFound:
      return "FDT node not found";
    case ErrorCode::kFdtPropertyNotFound:
      return "FDT property not found";
    case ErrorCode::kFdtParseFailed:
      return "FDT parse failed";
    case ErrorCode::kFdtInvalidPropertySize:
      return "Invalid FDT property size";
    case ErrorCode::kSpinLockRecursiveLock:
      return "Recursive spinlock detected";
    case ErrorCode::kSpinLockNotOwned:
      return "Spinlock not owned by current core";
    case ErrorCode::kMutexNoTaskContext:
      return "Mutex operation outside task context";
    case ErrorCode::kMutexRecursiveLock:
      return "Recursive mutex lock detected";
    case ErrorCode::kMutexNotOwned:
      return "Mutex not owned by current task";
    case ErrorCode::kMutexNotLocked:
      return "Mutex not locked";
    case ErrorCode::kVmAllocationFailed:
      return "Virtual memory allocation failed";
    case ErrorCode::kVmMapFailed:
      return "Virtual memory mapping failed";
    case ErrorCode::kVmUnmapFailed:
      return "Virtual memory unmapping failed";
    case ErrorCode::kVmInvalidPageTable:
      return "Invalid page table";
    case ErrorCode::kVmPageNotMapped:
      return "Page not mapped";
    case ErrorCode::kIpiTargetOutOfRange:
      return "IPI target CPU mask out of range";
    case ErrorCode::kIpiSendFailed:
      return "IPI send failed";
    case ErrorCode::kTaskNoCurrentTask:
      return "No current task";
    case ErrorCode::kTaskPidAllocationFailed:
      return "PID allocation failed";
    case ErrorCode::kTaskAllocationFailed:
      return "Task allocation failed";
    case ErrorCode::kTaskInvalidCloneFlags:
      return "Invalid clone flags";
    case ErrorCode::kTaskPageTableCloneFailed:
      return "Page table clone failed";
    case ErrorCode::kTaskKernelStackAllocationFailed:
      return "Kernel stack allocation failed";
    case ErrorCode::kTaskNoChildFound:
      return "No child process found";
    case ErrorCode::kTaskInvalidPid:
      return "Invalid PID";
    case ErrorCode::kSignalInvalidNumber:
      return "Invalid signal number";
    case ErrorCode::kSignalInvalidPid:
      return "Invalid PID for signal delivery";
    case ErrorCode::kSignalPermissionDenied:
      return "Signal delivery permission denied";
    case ErrorCode::kSignalUncatchable:
      return "Cannot catch or ignore this signal";
    case ErrorCode::kSignalTaskNotFound:
      return "Target task not found for signal delivery";
    case ErrorCode::kDeviceNotFound:
      return "Device not found";
    case ErrorCode::kDeviceAlreadyOpen:
      return "Device already open";
    case ErrorCode::kDeviceNotOpen:
      return "Device not open";
    case ErrorCode::kDeviceReadFailed:
      return "Device read failed";
    case ErrorCode::kDeviceWriteFailed:
      return "Device write failed";
    case ErrorCode::kDeviceIoctlFailed:
      return "Device ioctl failed";
    case ErrorCode::kDeviceMmapFailed:
      return "Device mmap failed";
    case ErrorCode::kDeviceNotSupported:
      return "Operation not supported by device";
    case ErrorCode::kDeviceBusy:
      return "Device busy";
    case ErrorCode::kDevicePermissionDenied:
      return "Device permission denied";
    case ErrorCode::kDeviceInvalidOffset:
      return "Invalid device offset";
    case ErrorCode::kDeviceBlockUnaligned:
      return "Block access not aligned to block size";
    case ErrorCode::kDeviceBlockOutOfRange:
      return "Block number out of device range";
    case ErrorCode::kDeviceFlushFailed:
      return "Device flush failed";
    case ErrorCode::kDeviceError:
      return "Device reported an error";
    case ErrorCode::kIoError:
      return "I/O operation failed";
    case ErrorCode::kNotSupported:
      return "Operation not supported";
    case ErrorCode::kTimeout:
      return "Operation timed out";
    // VirtIO 传输层错误 (0x820 - 0x82F)
    case ErrorCode::kInvalidMagic:
      return "Invalid MMIO magic value";
    case ErrorCode::kInvalidVersion:
      return "Unsupported virtio version";
    case ErrorCode::kInvalidDeviceId:
      return "Invalid device ID (device does not exist)";
    case ErrorCode::kTransportNotInitialized:
      return "Transport layer not initialized";
    case ErrorCode::kFeatureNegotiationFailed:
      return "Feature negotiation failed";
    // VirtIO 虚拟队列错误 (0x830 - 0x83F)
    case ErrorCode::kQueueNotAvailable:
      return "Queue not available (queue_num_max == 0)";
    case ErrorCode::kQueueAlreadyUsed:
      return "Queue already used";
    case ErrorCode::kQueueTooLarge:
      return "Requested queue size exceeds maximum";
    case ErrorCode::kNoFreeDescriptors:
      return "No free descriptors available";
    case ErrorCode::kInvalidDescriptor:
      return "Invalid descriptor index";
    case ErrorCode::kNoUsedBuffers:
      return "No used buffers to reclaim";
    case ErrorCode::kFsFileNotFound:
      return "File not found";
    case ErrorCode::kFsPermissionDenied:
      return "Filesystem permission denied";
    case ErrorCode::kFsNotADirectory:
      return "Not a directory";
    case ErrorCode::kFsIsADirectory:
      return "Is a directory";
    case ErrorCode::kFsFileExists:
      return "File already exists";
    case ErrorCode::kFsNoSpace:
      return "No space left on device";
    case ErrorCode::kFsMountFailed:
      return "Mount failed";
    case ErrorCode::kFsUnmountFailed:
      return "Unmount failed";
    case ErrorCode::kFsInvalidPath:
      return "Invalid path";
    case ErrorCode::kFsFdTableFull:
      return "File descriptor table full";
    case ErrorCode::kFsInvalidFd:
      return "Invalid file descriptor";
    case ErrorCode::kFsNotMounted:
      return "Filesystem not mounted";
    case ErrorCode::kFsReadOnly:
      return "Read-only filesystem";
    case ErrorCode::kFsCorrupted:
      return "Filesystem corrupted";
    case ErrorCode::kFsAlreadyMounted:
      return "Filesystem already mounted";
    case ErrorCode::kFsNotEmpty:
      return "Directory not empty";
    case ErrorCode::kBlkDeviceNotFound:
      return "Block device not found";
    case ErrorCode::kBlkReadFailed:
      return "Block read failed";
    case ErrorCode::kBlkWriteFailed:
      return "Block write failed";
    case ErrorCode::kBlkSectorOutOfRange:
      return "Sector out of range";
    case ErrorCode::kIrqChipInvalidIrq:
      return "IRQ number out of controller range";
    case ErrorCode::kIrqChipIrqNotEnabled:
      return "IRQ not enabled";
    case ErrorCode::kIrqChipAffinityFailed:
      return "Failed to set IRQ CPU affinity";
    case ErrorCode::kIrqChipIpiTimeout:
      return "IPI delivery timeout";
    case ErrorCode::kInvalidArgument:
      return "Invalid argument";
    case ErrorCode::kOutOfMemory:
      return "Out of memory";
    default:
      return "Unknown error";
  }
}

/// 错误类型，用于 std::expected
struct Error {
  ErrorCode code{ErrorCode::kSuccess};

  explicit constexpr Error(ErrorCode c) : code(c) {}

  /// @name 构造/析构函数
  /// @{
  Error() = default;
  Error(const Error&) = default;
  Error(Error&&) = default;
  auto operator=(const Error&) -> Error& = default;
  auto operator=(Error&&) -> Error& = default;
  ~Error() = default;
  /// @}

  [[nodiscard]] constexpr auto message() const -> const char* {
    return GetErrorMessage(code);
  }
};

/// std::expected 别名模板
template <typename T>
using Expected = std::expected<T, Error>;
