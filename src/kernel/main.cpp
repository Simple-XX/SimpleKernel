/**
 * @file main.cpp
 * @brief 内核入口
 * @author Zone.N (Zone.Niuzh@hotmail.com)
 * @version 1.0
 * @date 2023-07-15
 * @copyright MIT LICENSE
 * https://github.com/Simple-XX/SimpleKernel
 * @par change log:
 * <table>
 * <tr><th>Date<th>Author<th>Description
 * <tr><td>2023-07-15<td>Zone.N (Zone.Niuzh@hotmail.com)<td>创建文件
 * </table>
 */

#include <cstdint>

#include "arch.h"
#include "sk_cstdio"
#include "sk_iostream"
#include "kernel.h"
#include "kernel_log.hpp"
#include "sk_libcxx.h"

void _start(uint32_t argc, uint8_t *argv) {
  CppInit();
  main(argc, argv);
  CppDeInit();

  // 进入死循环
  while (1) {
    ;
  }
}

uint32_t main(uint32_t argc, uint8_t *argv) {
  // 架构相关初始化
  [[maybe_unused]] auto arch_init_ret = ArchInit(argc, argv);

  printf("Hello SimpleKernel\n");
  klog::Debug("Hello SimpleKernel\n");
  klog::Info("Hello SimpleKernel\n");
  klog::Warn("Hello SimpleKernel\n");
  klog::Err("Hello SimpleKernel\n");
  klog::debug << "Hello SimpleKernel\n";
  klog::info << "Hello SimpleKernel\n";
  klog::warn << "Hello SimpleKernel\n";
  klog::err << "Hello SimpleKernel\n";

  DumpStack();

  sk_std::cout << "Hello ostream" << sk_std::endl;

  InterruptInit(argc, argv);

  // 进入死循环
  while (1) {
    for (uint64_t i = 0; i < 99999999; i++) {
    }
    printf("-");
  }
  return 0;
}
