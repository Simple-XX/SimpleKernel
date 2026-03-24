/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cstdint>

/**
 * @brief 负责 crtbegin 的工作
 * @param argc
 *          riscv64: 启动核 id
 * @param argv 参数指针
 *          riscv64: dtb 地址
 * @return uint32_t 正常返回 0
 */
extern "C" [[maybe_unused]] [[noreturn]] void _start(int argc,
                                                     const char** argv);

/**
 * @brief 内核入口
 * @param argc 同 _start
 * @param argv 同 _start
 * @return int 正常返回 0
 */
auto main(int argc, const char** argv) -> int;

/// @brief 内存子系统初始化
auto MemoryInit() -> void;
/// @brief 多核内存子系统初始化
auto MemoryInitSMP() -> void;

/// @brief 设备子系统初始化
auto DeviceInit() -> void;

/// @brief 文件系统初始化
auto FileSystemInit() -> void;
