/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <elf.h>
#include <etl/singleton.h>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>

#include "expected.hpp"
#include "kernel_log.hpp"

/**
 * @brief ELF 文件相关
 */
class KernelElf {
 public:
  /// 符号表
  std::span<Elf64_Sym> symtab{};
  /// 字符串表
  uint8_t* strtab{nullptr};

  /**
   * @brief 获取 elf 文件大小
   * @return elf 文件大小
   */
  [[nodiscard]] auto GetElfSize() const -> size_t { return elf_.size(); }

  /// @name 构造/析构函数
  /// @{

  /**
   * @brief 构造函数
   * @param elf_addr elf 地址
   */
  explicit KernelElf(uint64_t elf_addr) {
    assert(elf_addr != 0U && "elf_addr is null");

    elf_ = std::span<uint8_t>(reinterpret_cast<uint8_t*>(elf_addr), EI_NIDENT);

    // 检查 elf 头数据
    CheckElfIdentity().or_else([](Error err) -> Expected<void> {
      klog::Err("KernelElf NOT valid ELF file: {}", err.message());
      while (true) {
        cpu_io::Pause();
      }
      return {};
    });

    ehdr_ = *reinterpret_cast<const Elf64_Ehdr*>(elf_.data());

    // 重新计算 elf 大小
    size_t max_size = EI_NIDENT;
    if (ehdr_.e_phoff != 0) {
      size_t ph_end = ehdr_.e_phoff + ehdr_.e_phnum * ehdr_.e_phentsize;
      if (ph_end > max_size) {
        max_size = ph_end;
      }
    }
    if (ehdr_.e_shoff != 0) {
      size_t sh_end = ehdr_.e_shoff + ehdr_.e_shnum * ehdr_.e_shentsize;
      if (sh_end > max_size) {
        max_size = sh_end;
      }
      const auto* shdrs =
          reinterpret_cast<const Elf64_Shdr*>(elf_.data() + ehdr_.e_shoff);
      for (int i = 0; i < ehdr_.e_shnum; ++i) {
        size_t section_end = shdrs[i].sh_offset + shdrs[i].sh_size;
        if (section_end > max_size) {
          max_size = section_end;
        }
      }
    }
    elf_ = std::span<uint8_t>(reinterpret_cast<uint8_t*>(elf_addr), max_size);

    phdr_ = std::span<Elf64_Phdr>(
        reinterpret_cast<Elf64_Phdr*>(elf_.data() + ehdr_.e_phoff),
        ehdr_.e_phnum);

    shdr_ = std::span<Elf64_Shdr>(
        reinterpret_cast<Elf64_Shdr*>(elf_.data() + ehdr_.e_shoff),
        ehdr_.e_shnum);

    const auto* shstrtab = reinterpret_cast<const char*>(elf_.data()) +
                           shdr_[ehdr_.e_shstrndx].sh_offset;
    for (auto shdr : shdr_) {
      if (strcmp(shstrtab + shdr.sh_name, ".symtab") == 0) {
        symtab = std::span<Elf64_Sym>(
            reinterpret_cast<Elf64_Sym*>(elf_.data() + shdr.sh_offset),
            (shdr.sh_size / sizeof(Elf64_Sym)));
      } else if (strcmp(shstrtab + shdr.sh_name, ".strtab") == 0) {
        strtab = elf_.data() + shdr.sh_offset;
      }
    }
  }

  KernelElf() = default;
  KernelElf(const KernelElf&) = default;
  KernelElf(KernelElf&&) = default;
  auto operator=(const KernelElf&) -> KernelElf& = default;
  auto operator=(KernelElf&&) -> KernelElf& = default;
  ~KernelElf() = default;
  /// @}

 protected:
  /// @name elf 文件相关
  /// @{
  std::span<uint8_t> elf_{};
  Elf64_Ehdr ehdr_{};
  std::span<Elf64_Phdr> phdr_{};
  std::span<Elf64_Shdr> shdr_{};
  /// @}

  /**
   * 检查 elf 标识
   * @return 成功返回 Expected<void>，失败返回错误
   */
  [[nodiscard]] auto CheckElfIdentity() const -> Expected<void> {
    return CheckElfMagic().and_then([this]() { return CheckElfClass(); });
  }

 private:
  /**
   * 检查 ELF magic number
   */
  [[nodiscard]] auto CheckElfMagic() const -> Expected<void> {
    if ((elf_[EI_MAG0] != ELFMAG0) || (elf_[EI_MAG1] != ELFMAG1) ||
        (elf_[EI_MAG2] != ELFMAG2) || (elf_[EI_MAG3] != ELFMAG3)) {
      return std::unexpected(Error(ErrorCode::kElfInvalidMagic));
    }
    return {};
  }

  /**
   * 检查 ELF class (32/64 bit)
   */
  [[nodiscard]] auto CheckElfClass() const -> Expected<void> {
    if (elf_[EI_CLASS] == ELFCLASS32) {
      return std::unexpected(Error(ErrorCode::kElfUnsupported32Bit));
    }
    if (elf_[EI_CLASS] != ELFCLASS64) {
      return std::unexpected(Error(ErrorCode::kElfInvalidClass));
    }
    return {};
  }
};

using KernelElfSingleton = etl::singleton<KernelElf>;
