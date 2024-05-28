
/**
 * @file load_elf.cpp
 * @brief 加载 elf 文件
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

#include "load_elf.h"

#include <array>
#include <cstring>
#include <memory>
#include <span>

#include "out_stream.hpp"

namespace {

/**
 * 将 char* 转换为 wchar_t*
 * @param dst 输出
 * @param src 输入
 * @return 字符数量
 */
auto Char2Wchar(wchar_t *dst, const char *src) -> size_t {
  size_t idx = 0;
  while (src[idx] != '\0') {
    dst[idx] = src[idx];
    idx++;
  }
  return idx;
}

/// 一个制表符大小
constexpr const uint32_t kOneTabSize = 8;
/// 两个制表符大小
constexpr const uint32_t kTwoTabSize = 16;
/// 两位数对齐
constexpr const uint32_t kAlignTwo = 10;

}  // namespace

Elf::Elf(wchar_t *kernel_image_filename) {
  EFI_STATUS status = EFI_SUCCESS;
  // 打开文件系统协议
  status = LibLocateProtocol(&FileSystemProtocol,
                             reinterpret_cast<void **>(&file_system_protocol_));
  if (EFI_ERROR(status)) {
    debug << L"Elf::Elf LibLocateProtocol failed: " << status
          << OutStream::endl;
    return;
  }

  // 打开根文件系统
  status = uefi_call_wrapper(file_system_protocol_->OpenVolume, 2,
                             file_system_protocol_, &root_file_system_);
  if (EFI_ERROR(status)) {
    debug << L"Elf::Elf OpenVolume failed: " << status << OutStream::endl;
    return;
  }

  // 打开 elf 文件
  status = uefi_call_wrapper(root_file_system_->Open, 5, root_file_system_,
                             &elf_, kernel_image_filename, EFI_FILE_MODE_READ,
                             EFI_FILE_READ_ONLY);

  if (EFI_ERROR(status)) {
    debug << L"Elf::Elf Open failed: " << status << OutStream::endl;
    return;
  }

  // 获取 elf 文件大小
  elf_file_size_ = GetFileSize();
  debug << L"Kernel file size: " << elf_file_size_ << OutStream::endl;

  // 分配 elf 文件缓存
  status = uefi_call_wrapper(gBS->AllocatePool, 3, EfiLoaderData,
                             elf_file_size_, &elf_file_buffer_);

  if (EFI_ERROR(status)) {
    debug << L"Elf::Elf AllocatePool failed: " << status << OutStream::endl;
    return;
  }

  // 将内核文件读入内存
  status = uefi_call_wrapper(elf_->Read, 3, (EFI_FILE *)elf_, &elf_file_size_,
                             elf_file_buffer_);
  if (EFI_ERROR(status)) {
    debug << L"Elf::Elf Read failed: " << status << OutStream::endl;
    return;
  }

  file_ = std::span<uint8_t>(static_cast<uint8_t *>(elf_file_buffer_),
                             elf_file_size_);

  // 检查 elf 头数据
  auto check_elf_identity_ret = CheckElfIdentity();
  if (!check_elf_identity_ret) {
    debug << L"Elf::Elf NOT valid ELF file" << OutStream::endl;
    return;
  }

  // 读取 ehdr
  GetEhdr();
  PrintEhdr();
  // 读取 phdr
  GetPhdr();
  PrintPhdr();
  // 读取 shdr
  GetShdr();
  PrintShdr();
}

Elf::~Elf() {
  EFI_STATUS status = EFI_SUCCESS;
  // 关闭 elf 文件
  status = uefi_call_wrapper(elf_->Close, 1, elf_);
  if (EFI_ERROR(status)) {
    debug << L"Elf::~Elf() Close failed: " << status << OutStream::endl;
    return;
  }
  /// @note elf_file_buffer 不会被释放
}

auto Elf::Load() const -> uintptr_t {
  auto ret = LoadProgramSections();
  if (!ret) {
    debug << L"Elf::Load() failed." << OutStream::endl;
    return 0;
  }
  return ehdr_.e_entry;
}

auto Elf::GetFileSize() const -> size_t {
  // 获取 elf 文件大小
  auto *elf_file_info = LibFileInfo(elf_);
  auto file_size = elf_file_info->FileSize;
  return file_size;
}

auto Elf::CheckElfIdentity() const -> bool {
  if ((file_[EI_MAG0] != ELFMAG0) || (file_[EI_MAG1] != ELFMAG1) ||
      (file_[EI_MAG2] != ELFMAG2) || (file_[EI_MAG3] != ELFMAG3)) {
    debug << L"Fatal Error: Invalid ELF header" << OutStream::endl;
    return false;
  }
  if (file_[EI_CLASS] == ELFCLASS32) {
    debug << L"Found 32bit executable but NOT SUPPORT" << OutStream::endl;
    return false;
  }
  if (file_[EI_CLASS] == ELFCLASS64) {
    debug << L"Found 64bit executable" << OutStream::endl;
  } else {
    debug << L"Fatal Error: Invalid executable" << OutStream::endl;
    return false;
  }
  return true;
}

void Elf::GetEhdr() {
  ehdr_ = *reinterpret_cast<const Elf64_Ehdr *>(file_.data());
}

void Elf::PrintEhdr() const {
  debug << L"  Magic:    ";
  for (auto idx : ehdr_.e_ident) {
    debug << OutStream::hex_x << idx << L" ";
  }
  debug << OutStream::endl;

  debug << L"  Class:                                ";
  switch (ehdr_.e_ident[EI_CLASS]) {
    case ELFCLASSNONE: {
      debug << L"Invalid class";
      break;
    }
    case ELFCLASS32: {
      debug << L"ELF32";
      break;
    }
    case ELFCLASS64: {
      debug << L"ELF64";
      break;
    }
    default: {
      debug << ehdr_.e_ident[EI_CLASS];
      break;
    }
  }
  debug << OutStream::endl;

  debug << L"  Data:                                 ";
  switch (ehdr_.e_ident[EI_DATA]) {
    case ELFDATANONE: {
      debug << L"Invalid data encoding";
      break;
    }
    case ELFDATA2LSB: {
      debug << L"2's complement, little endian";
      break;
    }
    case ELFDATA2MSB: {
      debug << L"2's complement, big endian";
      break;
    }
    default: {
      debug << ehdr_.e_ident[EI_DATA];
      break;
    }
  }
  debug << OutStream::endl;

  debug << L"  Version:                              "
        << ehdr_.e_ident[EI_VERSION] << L" ";
  switch (ehdr_.e_ident[EI_VERSION]) {
    case EV_NONE: {
      debug << L"Invalid ELF version";
      break;
    }
    case EV_CURRENT: {
      debug << L"Current version";
      break;
    }
    default: {
      debug << ehdr_.e_ident[EI_VERSION];
      break;
    }
  }
  debug << OutStream::endl;

  debug << L"  OS/ABI:                               ";
  switch (ehdr_.e_ident[EI_OSABI]) {
    case ELFOSABI_SYSV: {
      debug << L"UNIX System V ABI";
      break;
    }
    default: {
      debug << ehdr_.e_ident[EI_OSABI];
      break;
    }
  }
  debug << OutStream::endl;

  debug << L"  ABI Version:                          "
        << ehdr_.e_ident[EI_ABIVERSION] << OutStream::endl;

  debug << L"  Type:                                 ";
  switch (ehdr_.e_type) {
    case ET_NONE: {
      debug << L"No file type";
      break;
    }
    case ET_REL: {
      debug << L"Relocatable file";
      break;
    }
    case ET_EXEC: {
      debug << L"Executable file";
      break;
    }
    case ET_DYN: {
      debug << L"DYN (Shared object file)";
      break;
    }
    case ET_CORE: {
      debug << L"Core file";
      break;
    }
    default: {
      debug << ehdr_.e_type;
      break;
    }
  }
  debug << OutStream::endl;

  debug << L"  Machine:                              ";
  switch (ehdr_.e_machine) {
    case EM_X86_64: {
      debug << L"AMD x86-64 architecture";
      break;
    }
    case EM_RISCV: {
      debug << L"RISC-V";
      break;
    }
    case EM_AARCH64: {
      debug << L"ARM AARCH64";
      break;
    }
    default: {
      debug << ehdr_.e_machine;
      break;
    }
  }
  debug << OutStream::endl;

  debug << L"  Version:                              " << OutStream::hex_x
        << ehdr_.e_version << OutStream::endl;
  debug << L"  Entry point address:                  " << OutStream::hex_x
        << ehdr_.e_entry << OutStream::endl;
  debug << L"  Start of program headers:             " << ehdr_.e_phoff
        << L" (bytes into file)" << OutStream::endl;
  debug << L"  Start of section headers:             " << ehdr_.e_shoff
        << L" (bytes into file)" << OutStream::endl;
  debug << L"  Flags:                                " << OutStream::hex_x
        << ehdr_.e_flags << OutStream::endl;
  debug << L"  Size of this header:                  " << ehdr_.e_ehsize
        << L" (bytes)" << OutStream::endl;
  debug << L"  Size of program headers:              " << ehdr_.e_phentsize
        << L" (bytes)" << OutStream::endl;
  debug << L"  Number of program headers:            " << ehdr_.e_phnum
        << OutStream::endl;
  debug << L"  Size of section headers:              " << ehdr_.e_shentsize
        << L" (bytes)" << OutStream::endl;
  debug << L"  Number of section headers:            " << ehdr_.e_shnum
        << OutStream::endl;
  debug << L"  Section header string table index:    " << ehdr_.e_shstrndx
        << OutStream::endl;
}

void Elf::GetPhdr() {
  phdr_ = std::span<Elf64_Phdr>(
      reinterpret_cast<Elf64_Phdr *>(file_.data() + ehdr_.e_phoff),
      ehdr_.e_phnum);
}

void Elf::PrintPhdr() const {
  debug << L"\nProgram Headers:" << OutStream::endl;
  debug << L"  "
           L"Type\t\tOffset\t\tVirtAddr\tPhysAddr\tFileSiz\t\tMemSiz\t\tFlags"
           L"\tAlign"
        << OutStream::endl;
  for (uint64_t i = 0; i < ehdr_.e_phnum; i++) {
    switch (phdr_[i].p_type) {
      case PT_NULL: {
        debug << L"  NULL\t\t";
        break;
      }

      case PT_LOAD: {
        debug << L"  LOAD\t\t";
        break;
      }
      case PT_DYNAMIC: {
        debug << L"  DYNAMIC\t";
        break;
      }
      case PT_INTERP: {
        debug << L"  INTERP\t";
        break;
      }
      case PT_NOTE: {
        debug << L"  NOTE\t\t";
        break;
      }
      case PT_SHLIB: {
        debug << L"  SHLIB\t\t";
        break;
      }
      case PT_PHDR: {
        debug << L"  PHDR\t\t";
        break;
      }
      case PT_TLS: {
        debug << L"  TLS\t\t";
        break;
      }
      case PT_NUM: {
        debug << L"  NUM\t\t";
        break;
      }
      case PT_LOOS: {
        debug << L"  LOOS\t\t";
        break;
      }
      case PT_GNU_EH_FRAME: {
        debug << L"  GNU_EH_FRAME\t";
        break;
      }
      case PT_GNU_STACK: {
        debug << L"  GNU_STACK\t";
        break;
      }
      case PT_GNU_RELRO: {
        debug << L"  GNU_RELRO\t";
        break;
      }
      case PT_GNU_PROPERTY: {
        debug << L"  GNU_PROPERTY\t";
        break;
      }
      case PT_SUNWBSS: {
        debug << L"  SUNWBSS\t\t";
        break;
      }
      case PT_SUNWSTACK: {
        debug << L"  SUNWSTACK\t";
        break;
      }
      case PT_HIOS: {
        debug << L"  HIOS\t\t";
        break;
      }
      case PT_LOPROC: {
        debug << L"  LOPROC\t\t";
        break;
      }
      case PT_HIPROC: {
        debug << L"  HIPROC\t\t";
        break;
      }
      default: {
        debug << L"  Unknown " << OutStream::hex_X << phdr_[i].p_type << L"\t";
        break;
      }
    }

    debug << OutStream::hex_X << phdr_[i].p_offset << L"\t";
    debug << OutStream::hex_X << phdr_[i].p_vaddr << L"\t";
    debug << OutStream::hex_X << phdr_[i].p_paddr << L"\t";
    debug << OutStream::hex_X << phdr_[i].p_filesz << L"\t";
    debug << OutStream::hex_X << phdr_[i].p_memsz << L"\t";

    switch (phdr_[i].p_flags) {
      case PF_X: {
        debug << L"E\t";
        break;
      }
      case PF_W: {
        debug << L"W\t";
        break;
      }
      case PF_R: {
        debug << L"R\t";
        break;
      }
      case PF_MASKOS: {
        debug << L"OS-specific\t";
        break;
      }
      case PF_MASKPROC: {
        debug << L"Processor-specific\t";
        break;
      }
      case (PF_X | PF_R): {
        debug << L"R E\t";
        break;
      }
      case (PF_W | PF_R): {
        debug << L"RW\t";
        break;
      }
      default: {
        debug << L"Unknown " << OutStream::hex_x << phdr_[i].p_flags << L"\t";
        break;
      }
    }
    debug << OutStream::hex_x << phdr_[i].p_align << OutStream::endl;
  }
}

void Elf::GetShdr() {
  shdr_ = std::span<Elf64_Shdr>(
      reinterpret_cast<Elf64_Shdr *>(file_.data() + ehdr_.e_shoff),
      ehdr_.e_shnum);
  // 将 shstrtab 的内容复制到 shstrtab_buf 中

  memcpy(shstrtab_buf_.data(), file_.data() + shdr_[ehdr_.e_shstrndx].sh_offset,
         shdr_[ehdr_.e_shstrndx].sh_size);
}

void Elf::PrintShdr() const {
  debug << L"\nSection Headers:" << OutStream::endl;
  debug << L" [Nr] "
           L"Name\t\t\tType\t\tAddress\t\tOffset\t\tSize\t\tEntSize\t\tFl"
           L"ags\tLink\tInfo\tAlign"
        << OutStream::endl;
  for (uint64_t i = 0; i < ehdr_.e_shnum; i++) {
    debug << L" [";
    // 对齐
    if (i < kAlignTwo) {
      debug << L" ";
    }
    debug << i << L"] ";

    std::array<wchar_t, kSectionBufferSize> buf = {0};
    auto char2wchar_ret =
        Char2Wchar(buf.data(), reinterpret_cast<const char *>(
                                   shstrtab_buf_.data() + shdr_[i].sh_name));
    debug << (const wchar_t *)buf.data() << L"\t";

    if (char2wchar_ret <= kTwoTabSize) {
      debug << L"\t";
    }
    if (char2wchar_ret <= kOneTabSize) {
      debug << L"\t";
    }
    if (char2wchar_ret <= 1) {
      debug << L"\t";
    }
    switch (shdr_[i].sh_type) {
      case SHT_NULL: {
        debug << L"NULL\t\t";
        break;
      }
      case SHT_PROGBITS: {
        debug << L"PROGBITS\t";
        break;
      }
      case SHT_SYMTAB: {
        debug << L"SYMTAB\t\t";
        break;
      }
      case SHT_STRTAB: {
        debug << L"STRTAB\t\t";
        break;
      }
      case SHT_RELA: {
        debug << L"RELA\t\t";
        break;
      }
      case SHT_HASH: {
        debug << L"HASH\t\t";
        break;
      }
      case SHT_DYNAMIC: {
        debug << L"DYNAMIC\t\t";
        break;
      }
      case SHT_NOTE: {
        debug << L"NOTE\t\t";
        break;
      }
      case SHT_NOBITS: {
        debug << L"NOBITS\t\t";
        break;
      }
      case SHT_REL: {
        debug << L"REL\t\t";
        break;
      }
      case SHT_SHLIB: {
        debug << L"SHLIB\t\t";
        break;
      }
      case SHT_DYNSYM: {
        debug << L"DYNSYM\t\t";
        break;
      }
      case SHT_INIT_ARRAY: {
        debug << L"INIT_ARRAY\t";
        break;
      }
      case SHT_FINI_ARRAY: {
        debug << L"FINI_ARRAY\t";
        break;
      }
      case SHT_PREINIT_ARRAY: {
        debug << L"PREINIT_ARRAY\t\t";
        break;
      }
      case SHT_GROUP: {
        debug << L"GROUP\t\t";
        break;
      }
      case SHT_SYMTAB_SHNDX: {
        debug << L"SYMTAB_SHNDX\t\t";
        break;
      }
      case SHT_RELR: {
        debug << L"RELR\t\t";
        break;
      }
      case SHT_NUM: {
        debug << L"NUM\t\t";
        break;
      }
      case SHT_LOOS: {
        debug << L"LOOS\t\t";
        break;
      }
      case SHT_GNU_ATTRIBUTES: {
        debug << L"GNU_ATTRIBUTE\t\t";
        break;
      }
      case SHT_GNU_HASH: {
        debug << L"GNU_HASH\t";
        break;
      }
      case SHT_GNU_LIBLIST: {
        debug << L"GNU_LIBLIST\t\t";
        break;
      }
      case SHT_CHECKSUM: {
        debug << L"CHECKSUM\t\t";
        break;
      }
      case SHT_SUNW_move: {
        debug << L"SUNW_move\t\t";
        break;
      }
      case SHT_SUNW_COMDAT: {
        debug << L"SUNW_COMDAT\t\t";
        break;
      }
      case SHT_SUNW_syminfo: {
        debug << L"SUNW_syminfo\t\t";
        break;
      }
      case SHT_GNU_verdef: {
        debug << L"GNU_verdef\t\t";
        break;
      }
      case SHT_GNU_verneed: {
        debug << L"GNU_verneed\t";
        break;
      }
      case SHT_GNU_versym: {
        debug << L"GNU_versym\t";
        break;
      }
      case SHT_LOPROC: {
        debug << L"LOPROC\t\t";
        break;
      }
      case SHT_HIPROC: {
        debug << L"HIPROC\t\t";
        break;
      }
      case SHT_LOUSER: {
        debug << L"LOUSER\t\t";
        break;
      }
      case SHT_HIUSER: {
        debug << L"HIUSER\t\t";
        break;
      }
      default: {
        debug << L"Unknown " << OutStream::hex_X << shdr_[i].sh_type << L"\t";

        break;
      }
    }

    debug << OutStream::hex_X << shdr_[i].sh_addr << L"\t";
    debug << OutStream::hex_x << shdr_[i].sh_offset << L"\t\t";
    debug << OutStream::hex_X << shdr_[i].sh_size << L"\t";
    debug << OutStream::hex_X << shdr_[i].sh_entsize << L"\t";

    switch (shdr_[i].sh_flags) {
      case 0: {
        debug << L"0\t";
        break;
      }
      case SHF_WRITE: {
        debug << L"WRITE\t";
        break;
      }
      case SHF_ALLOC: {
        debug << L"A\t";
        break;
      }
      case SHF_EXECINSTR: {
        debug << L"EXECINSTR\t";
        break;
      }
      case SHF_MERGE: {
        debug << L"MERGE\t";
        break;
      }
      case SHF_STRINGS: {
        debug << L"STRINGS\t";
        break;
      }
      case SHF_INFO_LINK: {
        debug << L"INFO_LINK\t";
        break;
      }
      case SHF_LINK_ORDER: {
        debug << L"LINK_ORDER\t";
        break;
      }
      case SHF_OS_NONCONFORMING: {
        debug << L"OS_NONCONFORMING\t";
        break;
      }
      case SHF_GROUP: {
        debug << L"GROUP\t";
        break;
      }
      case SHF_TLS: {
        debug << L"TLS\t";
        break;
      }
      case SHF_COMPRESSED: {
        debug << L"COMPRESSED\t";
        break;
      }
      case SHF_MASKOS: {
        debug << L"MASKOS\t";
        break;
      }
      case SHF_MASKPROC: {
        debug << L"MASKPROC\t";
        break;
      }
      case SHF_GNU_RETAIN: {
        debug << L"GNU_RETAIN\t";
        break;
      }
      case SHF_ORDERED: {
        debug << L"ORDERED\t";
        break;
      }
      case SHF_EXCLUDE: {
        debug << L"EXCLUDE\t";
        break;
      }
      case (SHF_WRITE | SHF_ALLOC): {
        debug << L"WA\t";
        break;
      }
      case (SHF_ALLOC | SHF_MERGE): {
        debug << L"AM\t";
        break;
      }
      case (SHF_ALLOC | SHF_EXECINSTR): {
        debug << L"AX\t";
        break;
      }
      case (SHF_MERGE | SHF_STRINGS): {
        debug << L"MS\t";
        break;
      }
      default: {
        debug << L"Unknown " << OutStream::hex_X << shdr_[i].sh_flags << L"\t";
        break;
      }
    }

    debug << shdr_[i].sh_link << L"\t";
    debug << shdr_[i].sh_info << L"\t";
    debug << shdr_[i].sh_addralign << L"\t";
    debug << OutStream::endl;
  }
}

bool Elf::LoadSections(const Elf64_Phdr &phdr) const {
  EFI_STATUS status = EFI_SUCCESS;
  void *data = nullptr;
  // 计算使用的内存页数
  auto section_page_count = EFI_SIZE_TO_PAGES(phdr.p_memsz);

  // 设置文件偏移到 p_offset
  status = uefi_call_wrapper(elf_->SetPosition, 2, elf_, phdr.p_offset);
  if (EFI_ERROR(status)) {
    debug << L"Elf::LoadSections SetPosition failed: " << status
          << OutStream::endl;
    return false;
  }
  status = uefi_call_wrapper(gBS->AllocatePages, 4, AllocateAddress,
                             EfiLoaderData, section_page_count,
                             (EFI_PHYSICAL_ADDRESS *)&phdr.p_paddr);
  if (EFI_ERROR(status)) {
    debug << L"Elf::LoadSections AllocatePages AllocateAddress failed: "
          << status << L", phdr.p_paddr: " << OutStream::hex_X << phdr.p_paddr
          << OutStream::endl;
    return false;
  }

  if (phdr.p_filesz > 0) {
    auto buffer_read_size = phdr.p_filesz;
    // 为 program_data 分配内存
    status = uefi_call_wrapper(gBS->AllocatePool, 3, EfiLoaderCode,
                               buffer_read_size, (void **)&data);
    if (EFI_ERROR(status)) {
      debug << L"Elf::LoadSections AllocatePool failed: " << status
            << OutStream::endl;
      return false;
    }
    // 读数据
    status =
        uefi_call_wrapper(elf_->Read, 3, elf_, &buffer_read_size, (void *)data);
    if (EFI_ERROR(status)) {
      debug << L"Elf::LoadSections Read failed: " << status << OutStream::endl;
      return false;
    }

    // 将读出来的数据复制到其对应的物理地址
    uefi_call_wrapper(gBS->CopyMem, 3, reinterpret_cast<void *>(phdr.p_paddr),
                      data, phdr.p_filesz);

    // 释放 program_data
    status = uefi_call_wrapper(gBS->FreePool, 1, data);
    if (EFI_ERROR(status)) {
      debug << L"Elf::LoadSections FreePool failed: " << status
            << OutStream::endl;
      return false;
    }
  }

  // 计算填充大小
  auto *zero_fill_start =
      reinterpret_cast<void *>(phdr.p_paddr + phdr.p_filesz);
  auto zero_fill_count = phdr.p_memsz - phdr.p_filesz;
  if (zero_fill_count > 0) {
    debug << L"Debug: Zero-filling " << zero_fill_count
          << L" bytes at address '" << OutStream::hex_x << zero_fill_start
          << L"'" << OutStream::endl;

    // 将填充部分置 0
    uefi_call_wrapper(gBS->SetMem, 3, zero_fill_start, zero_fill_count, 0);
  }
  return true;
}

bool Elf::LoadProgramSections() const {
  for (uint64_t i = 0; i < ehdr_.e_phnum; i++) {
    if (phdr_[i].p_type != PT_LOAD) {
      continue;
    }
    auto load_sections_ret = LoadSections(phdr_[i]);
    if (!load_sections_ret) {
      debug << L"Elf::LoadProgramSections() LoadSections failed " << i
            << OutStream::endl;
      return false;
    }
  }
  return true;
}

void Elf::LoadKernelImage() {
  for (auto &i : phdr_)
    ;
}
