
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

#include "ostream.hpp"

namespace {

/**
 * 将 char* 转换为 wchar_t*
 * @param _dst 输出
 * @param _src 输入
 * @return 字符数量
 */
auto char2wchar(wchar_t *_dst, const char *_src) -> size_t {
  size_t idx = 0;
  while (_src[idx] != '\0') {
    _dst[idx] = _src[idx];
    idx++;
  }
  return idx;
}

/// 一个制表符大小
constexpr const uint32_t ONE_TAB_SIZE = 8;
/// 两个制表符大小
constexpr const uint32_t TWO_TAB_SIZE = 16;
/// 两位数对齐
constexpr const uint32_t ALIGN_TWO = 10;

}  // namespace

Elf::Elf(wchar_t *_kernel_image_filename) {
  EFI_STATUS status = EFI_SUCCESS;
  // 打开文件系统协议
  status = LibLocateProtocol(&FileSystemProtocol,
                             reinterpret_cast<void **>(&file_system_protocol));
  if (EFI_ERROR(status)) {
    debug << L"LibLocateProtocol failed: " << status << ostream::endl;
    throw std::runtime_error("EFI_ERROR(status)");
  }

  // 打开根文件系统
  status = uefi_call_wrapper(file_system_protocol->OpenVolume, 2,
                             file_system_protocol, &root_file_system);
  if (EFI_ERROR(status)) {
    debug << L"OpenVolume failed: " << status << ostream::endl;
    throw std::runtime_error("EFI_ERROR(status)");
  }

  // 打开 elf 文件
  status = uefi_call_wrapper(root_file_system->Open, 5, root_file_system, &elf,
                             _kernel_image_filename, EFI_FILE_MODE_READ,
                             EFI_FILE_READ_ONLY);

  if (EFI_ERROR(status)) {
    debug << L"Open failed: " << status << ostream::endl;
    throw std::runtime_error("EFI_ERROR(status)");
  }

  // 获取 elf 文件大小
  try {
    elf_file_size = get_file_size();
  } catch (std::runtime_error &_e) {
    debug << L"get_file_size failed: " << _e.what() << ostream::endl;
    throw std::runtime_error(_e.what());
  }
  debug << L"Kernel file size: " << elf_file_size << ostream::endl;

  // 分配 elf 文件缓存
  status = uefi_call_wrapper(gBS->AllocatePool, 3, EfiLoaderData, elf_file_size,
                             &elf_file_buffer);

  if (EFI_ERROR(status)) {
    debug << L"AllocatePool failed: " << status << ostream::endl;
    throw std::runtime_error("EFI_ERROR(status)");
  }

  // 将内核文件读入内存
  status = uefi_call_wrapper(elf->Read, 3, (EFI_FILE *)elf, &elf_file_size,
                             elf_file_buffer);
  if (EFI_ERROR(status)) {
    debug << L"Read failed: " << status << ostream::endl;
    throw std::runtime_error("EFI_ERROR(status)");
  }

  file = std::span<uint8_t>(static_cast<uint8_t *>(elf_file_buffer),
                            elf_file_size);

  // 检查 elf 头数据
  auto check_elf_identity_ret = check_elf_identity();
  if (!check_elf_identity_ret) {
    debug << L"NOT valid ELF file" << ostream::endl;
    throw std::runtime_error("check_elf_identity_ret == false");
  }

  // 读取 ehdr
  get_ehdr();
  print_ehdr();
  // 读取 phdr
  get_phdr();
  print_phdr();
  // 读取 shdr
  get_shdr();
  print_shdr();
}

Elf::~Elf() {
  try {
    EFI_STATUS status = EFI_SUCCESS;
    // 关闭 elf 文件
    status = uefi_call_wrapper(elf->Close, 1, elf);
    if (EFI_ERROR(status)) {
      debug << L"Close failed: " << status << ostream::endl;
      throw std::runtime_error("EFI_ERROR(status)");
    }
    /// @note elf_file_buffer 不会被释放
  } catch (std::runtime_error &_e) {
    debug << L"~Elf failed: " << _e.what() << ostream::endl;
  }
}

auto Elf::load_kernel_image() const -> uint64_t {
  uintptr_t image_base = 0;
  uintptr_t image_begin = 0;
  try {
    //      load_program_sections();
    size_t size = 0;
    for (uint64_t i = 0; i < ehdr.e_phnum; i++) {
      if (phdr[i].p_type != PT_LOAD) {
        continue;
      }
      phdr[i].p_vaddr;
      size += phdr[i].p_memsz;
    }
    auto section_page_count = EFI_SIZE_TO_PAGES(size);
    auto status =
        uefi_call_wrapper(gBS->AllocatePages, 4, AllocateAnyPages,
                          EfiLoaderCode, section_page_count, &image_base);
    if (EFI_ERROR(status)) {
      debug << L"AllocatePages failed: " << status << ostream::endl;
      throw std::runtime_error("EFI_ERROR(status)");
    }

    for (auto &i : phdr) {
      if (i.p_type != PT_LOAD) {
        continue;
      }
      memcpy((void *)(image_base + i.p_vaddr), file.data() + i.p_offset,
             i.p_memsz);
    }

  } catch (std::runtime_error &_e) {
    debug << L"load_kernel_image: " << _e.what() << ostream::endl;
  }
  debug << L"load_kernel_image: " << ostream::hex_X << image_base << L" "
        << ostream::hex_X << ehdr.e_entry << L" " << ostream::hex_X
        << image_begin << ostream::endl;

  return image_base + ehdr.e_entry - image_begin;
}

auto Elf::load() const -> uintptr_t {
  // 记录 AllocatePages 分配出的物理地址
  uintptr_t image_base = 0;
  // 计算需要的内存页
  auto section_page_count = EFI_SIZE_TO_PAGES(elf_file_size);
  //  将整个 elf 文件映射到内存，方便后续读取
  auto status =
      uefi_call_wrapper(gBS->AllocatePages, 4, AllocateAnyPages, EfiLoaderCode,
                        section_page_count, &image_base);
  if (EFI_ERROR(status)) {
    debug << L"AllocatePages failed: " << status << ostream::endl;
    throw std::runtime_error("EFI_ERROR(status)");
  }
  // 将 elf 复制到分配的物理内存中
  std::memcpy(reinterpret_cast<void *>(image_base), file.data(),
              section_page_count * EFI_PAGE_SIZE);

  debug << L"AllocatePages section_page_count: " << section_page_count
        << L" image_base: " << ostream::hex_X << image_base << ostream::endl;
  return image_base + ehdr.e_entry;
}

auto Elf::get_file_size() const -> size_t {
  // 获取 elf 文件大小
  auto *elf_file_info = LibFileInfo(elf);
  auto file_size = elf_file_info->FileSize;
  return file_size;
}

auto Elf::check_elf_identity() const -> bool {
  if ((file[EI_MAG0] != ELFMAG0) || (file[EI_MAG1] != ELFMAG1) ||
      (file[EI_MAG2] != ELFMAG2) || (file[EI_MAG3] != ELFMAG3)) {
    debug << L"Fatal Error: Invalid ELF header" << ostream::endl;
    return false;
  }
  if (file[EI_CLASS] == ELFCLASS32) {
    debug << L"Found 32bit executable but NOT SUPPORT" << ostream::endl;
    return false;
  }
  if (file[EI_CLASS] == ELFCLASS64) {
    debug << L"Found 64bit executable" << ostream::endl;
  } else {
    debug << L"Fatal Error: Invalid executable" << ostream::endl;
    return false;
  }
  return true;
}

void Elf::get_ehdr() {
  ehdr = *reinterpret_cast<const Elf64_Ehdr *>(file.data());
}

void Elf::print_ehdr() const {
  debug << L"  Magic:    ";
  for (auto idx : ehdr.e_ident) {
    debug << ostream::hex_x << idx << L" ";
  }
  debug << ostream::endl;

  debug << L"  Class:                                ";
  switch (ehdr.e_ident[EI_CLASS]) {
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
      debug << ehdr.e_ident[EI_CLASS];
      break;
    }
  }
  debug << ostream::endl;

  debug << L"  Data:                                 ";
  switch (ehdr.e_ident[EI_DATA]) {
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
      debug << ehdr.e_ident[EI_DATA];
      break;
    }
  }
  debug << ostream::endl;

  debug << L"  Version:                              "
        << ehdr.e_ident[EI_VERSION] << L" ";
  switch (ehdr.e_ident[EI_VERSION]) {
    case EV_NONE: {
      debug << L"Invalid ELF version";
      break;
    }
    case EV_CURRENT: {
      debug << L"Current version";
      break;
    }
    default: {
      debug << ehdr.e_ident[EI_VERSION];
      break;
    }
  }
  debug << ostream::endl;

  debug << L"  OS/ABI:                               ";
  switch (ehdr.e_ident[EI_OSABI]) {
    case ELFOSABI_SYSV: {
      debug << L"UNIX System V ABI";
      break;
    }
    default: {
      debug << ehdr.e_ident[EI_OSABI];
      break;
    }
  }
  debug << ostream::endl;

  debug << L"  ABI Version:                          "
        << ehdr.e_ident[EI_ABIVERSION] << ostream::endl;

  debug << L"  Type:                                 ";
  switch (ehdr.e_type) {
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
      debug << ehdr.e_type;
      break;
    }
  }
  debug << ostream::endl;

  debug << L"  Machine:                              ";
  switch (ehdr.e_machine) {
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
      debug << ehdr.e_machine;
      break;
    }
  }
  debug << ostream::endl;

  debug << L"  Version:                              " << ostream::hex_x
        << ehdr.e_version << ostream::endl;
  debug << L"  Entry point address:                  " << ostream::hex_x
        << ehdr.e_entry << ostream::endl;
  debug << L"  Start of program headers:             " << ehdr.e_phoff
        << L" (bytes into file)" << ostream::endl;
  debug << L"  Start of section headers:             " << ehdr.e_shoff
        << L" (bytes into file)" << ostream::endl;
  debug << L"  Flags:                                " << ostream::hex_x
        << ehdr.e_flags << ostream::endl;
  debug << L"  Size of this header:                  " << ehdr.e_ehsize
        << L" (bytes)" << ostream::endl;
  debug << L"  Size of program headers:              " << ehdr.e_phentsize
        << L" (bytes)" << ostream::endl;
  debug << L"  Number of program headers:            " << ehdr.e_phnum
        << ostream::endl;
  debug << L"  Size of section headers:              " << ehdr.e_shentsize
        << L" (bytes)" << ostream::endl;
  debug << L"  Number of section headers:            " << ehdr.e_shnum
        << ostream::endl;
  debug << L"  Section header string table index:    " << ehdr.e_shstrndx
        << ostream::endl;
}

void Elf::get_phdr() {
  phdr = std::span<Elf64_Phdr>(
      reinterpret_cast<Elf64_Phdr *>(file.data() + ehdr.e_phoff), ehdr.e_phnum);
}

void Elf::print_phdr() const {
  debug << L"\nProgram Headers:" << ostream::endl;
  debug << L"  "
           L"Type\t\tOffset\t\tVirtAddr\tPhysAddr\tFileSiz\t\tMemSiz\t\tFlags"
           L"\tAlign"
        << ostream::endl;
  for (uint64_t i = 0; i < ehdr.e_phnum; i++) {
    switch (phdr[i].p_type) {
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
        debug << L"  Unknown " << ostream::hex_X << phdr[i].p_type << L"\t";
        break;
      }
    }

    debug << ostream::hex_X << phdr[i].p_offset << L"\t";
    debug << ostream::hex_X << phdr[i].p_vaddr << L"\t";
    debug << ostream::hex_X << phdr[i].p_paddr << L"\t";
    debug << ostream::hex_X << phdr[i].p_filesz << L"\t";
    debug << ostream::hex_X << phdr[i].p_memsz << L"\t";

    switch (phdr[i].p_flags) {
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
        debug << L"Unknown " << ostream::hex_x << phdr[i].p_flags << L"\t";
        break;
      }
    }
    debug << ostream::hex_x << phdr[i].p_align << ostream::endl;
  }
}

void Elf::get_shdr() {
  shdr = std::span<Elf64_Shdr>(
      reinterpret_cast<Elf64_Shdr *>(file.data() + ehdr.e_shoff), ehdr.e_shnum);
  // 将 shstrtab 的内容复制到 shstrtab_buf 中

  memcpy(shstrtab_buf.data(), file.data() + shdr[ehdr.e_shstrndx].sh_offset,
         shdr[ehdr.e_shstrndx].sh_size);
}

void Elf::print_shdr() const {
  debug << L"\nSection Headers:" << ostream::endl;
  debug << L" [Nr] "
           L"Name\t\t\tType\t\tAddress\t\tOffset\t\tSize\t\tEntSize\t\tFl"
           L"ags\tLink\tInfo\tAlign"
        << ostream::endl;
  for (uint64_t i = 0; i < ehdr.e_shnum; i++) {
    debug << L" [";
    // 对齐
    if (i < ALIGN_TWO) {
      debug << L" ";
    }
    debug << i << L"] ";

    std::array<wchar_t, SECTION_BUF_SIZE> buf = {0};
    auto char2wchar_ret = char2wchar(
        buf.data(),
        reinterpret_cast<const char *>(shstrtab_buf.data() + shdr[i].sh_name));
    debug << (const wchar_t *)buf.data() << L"\t";

    if (char2wchar_ret <= TWO_TAB_SIZE) {
      debug << L"\t";
    }
    if (char2wchar_ret <= ONE_TAB_SIZE) {
      debug << L"\t";
    }
    if (char2wchar_ret <= 1) {
      debug << L"\t";
    }
    switch (shdr[i].sh_type) {
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
        debug << L"Unknown " << ostream::hex_X << shdr[i].sh_type << L"\t";

        break;
      }
    }

    debug << ostream::hex_X << shdr[i].sh_addr << L"\t";
    debug << ostream::hex_x << shdr[i].sh_offset << L"\t\t";
    debug << ostream::hex_X << shdr[i].sh_size << L"\t";
    debug << ostream::hex_X << shdr[i].sh_entsize << L"\t";

    switch (shdr[i].sh_flags) {
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
        debug << L"Unknown " << ostream::hex_X << shdr[i].sh_flags << L"\t";
        break;
      }
    }

    debug << shdr[i].sh_link << L"\t";
    debug << shdr[i].sh_info << L"\t";
    debug << shdr[i].sh_addralign << L"\t";
    debug << ostream::endl;
  }
}

void Elf::load_sections(const Elf64_Phdr &_phdr) const {
  EFI_STATUS status = EFI_SUCCESS;
  void *data = nullptr;
  // 计算使用的内存页数
  auto section_page_count = EFI_SIZE_TO_PAGES(_phdr.p_memsz);

  // 设置文件偏移到 p_offset
  status = uefi_call_wrapper(elf->SetPosition, 2, elf, _phdr.p_offset);
  if (EFI_ERROR(status)) {
    debug << L"SetPosition failed: " << status << ostream::endl;
    throw std::runtime_error("memory_map == nullptr");
  }
  uintptr_t aaa = 0;
  // status = uefi_call_wrapper(gBS->AllocatePages, 4, AllocateAddress,
  //                            EfiLoaderData, section_page_count,
  //                            (EFI_PHYSICAL_ADDRESS*)&_phdr.p_paddr);
  status = uefi_call_wrapper(gBS->AllocatePages, 4, AllocateAnyPages,
                             EfiLoaderData, section_page_count, &aaa);
  debug << L"_phdr.p_paddr: [" << status << L"] [" << section_page_count
        << L"] " << ostream::hex_X << aaa << ostream::endl;
  if (EFI_ERROR(status)) {
    debug << L"AllocatePages AllocateAddress failed: " << status
          << ostream::endl;
    throw std::runtime_error("EFI_ERROR(status)");
  }

  if (_phdr.p_filesz > 0) {
    auto buffer_read_size = _phdr.p_filesz;
    // 为 program_data 分配内存
    status = uefi_call_wrapper(gBS->AllocatePool, 3, EfiLoaderCode,
                               buffer_read_size, (void **)&data);
    if (EFI_ERROR(status)) {
      debug << L"AllocatePool failed: " << status << ostream::endl;
      throw std::runtime_error("EFI_ERROR(status)");
    }
    // 读数据
    status =
        uefi_call_wrapper(elf->Read, 3, elf, &buffer_read_size, (void *)data);
    if (EFI_ERROR(status)) {
      debug << L"Read failed: " << status << ostream::endl;
      throw std::runtime_error("EFI_ERROR(status)");
    }

    // 将读出来的数据复制到其对应的物理地址
    uefi_call_wrapper(gBS->CopyMem, 3,
                      reinterpret_cast<void *>(aaa + _phdr.p_paddr), data,
                      _phdr.p_filesz);

    // 释放 program_data
    status = uefi_call_wrapper(gBS->FreePool, 1, data);
    if (EFI_ERROR(status)) {
      debug << L"FreePool failed: " << status << ostream::endl;
      throw std::runtime_error("EFI_ERROR(status)");
    }
  }

  // 计算填充大小
  auto *zero_fill_start =
      reinterpret_cast<void *>(aaa + _phdr.p_paddr + _phdr.p_filesz);
  auto zero_fill_count = _phdr.p_memsz - _phdr.p_filesz;
  if (zero_fill_count > 0) {
    debug << L"Debug: Zero-filling " << zero_fill_count
          << L" bytes at address '" << ostream::hex_x << zero_fill_start << L"'"
          << ostream::endl;

    // 将填充部分置 0
    uefi_call_wrapper(gBS->SetMem, 3, zero_fill_start, zero_fill_count, 0);
  }
}

void Elf::load_program_sections() const {
  uint64_t loaded = 0;
  for (uint64_t i = 0; i < ehdr.e_phnum; i++) {
    if (phdr[i].p_type != PT_LOAD) {
      continue;
    }
    load_sections(phdr[i]);
    loaded++;
  }

  if (loaded == 0) {
    debug << L"Fatal Error: No loadable program segments found in Kernel image "
          << ostream::endl;
    throw std::runtime_error("loaded == 0");
  }
}
