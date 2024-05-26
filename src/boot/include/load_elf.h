
/**
 * @file load_elf.h
 * @brief load_elf h
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

#ifndef SIMPLEKERNEL_SRC_BOOT_INCLUDE_LOAD_ELF_H_
#define SIMPLEKERNEL_SRC_BOOT_INCLUDE_LOAD_ELF_H_

#include <elf.h>

#include <array>
#include <span>
#include <utility>

#ifdef __cplusplus
extern "C" {
#endif

#include "efi.h"
#include "efilib.h"

#ifdef __cplusplus
}
#endif

/**
 * 图形相关
 */
class Graphics {
 public:
  /**
   * 构造函数
   */
  Graphics();

  /**
   * 析构函数
   */
  ~Graphics() = default;

  /// @name 不使用的构造函数
  /// @{
  Graphics(const Graphics &) = delete;
  Graphics(Graphics &&) = delete;
  auto operator=(const Graphics &) -> Graphics & = delete;
  auto operator=(Graphics &&) -> Graphics & = delete;
  /// @}

  /**
   * 设置图形模式
   * @param _format 图形像素格式，默认为 PixelBlueGreenRedReserved8BitPerColor
   * @param _width 宽度，默认为 1920
   * @param _height 高度，默认为 1080
   */
  void set_mode(
      EFI_GRAPHICS_PIXEL_FORMAT _format = PixelRedGreenBlueReserved8BitPerColor,
      uint32_t _width = DEFAULT_WIDTH, uint32_t _height = DEFAULT_HEIGHT) const;

  [[nodiscard]] auto get_framebuffer() const -> std::pair<uint64_t, uint32_t>;

  /**
   * 输出图形信息
   */
  void print_info() const;

 private:
  /// @name 默认分辨率
  /// @{
  static constexpr const uint32_t DEFAULT_WIDTH = 800;
  static constexpr const uint32_t DEFAULT_HEIGHT = 600;
  /// @}
  /// 图形输出协议
  EFI_GRAPHICS_OUTPUT_PROTOCOL *gop_ = nullptr;
};

/**
 * 内存相关
 */
class Memory {
 public:
  /**
   * 构造函数
   */
  Memory();

  /**
   * 析构函数
   */
  ~Memory() = default;

  /// @name 不使用的构造函数
  /// @{
  Memory(const Memory &) = delete;
  Memory(Memory &&) = delete;
  auto operator=(const Memory &) -> Memory & = delete;
  auto operator=(Memory &&) -> Memory & = delete;
  /// @}

  /**
   * 输出内存映射信息
   */
  void print_info();

 private:
  /// @name 内存映射信息
  /// @{
  uint64_t desc_count_ = 0;
  EFI_MEMORY_DESCRIPTOR *memory_map_ = nullptr;
  uint64_t map_key_ = 0;
  uint64_t desc_size_ = 0;
  uint32_t desc_version_ = 0;
  /// @}

  /**
   * 更新内存映射信息
   * @return 失败返回 false
   */
  bool flush_desc();
};

/**
 * elf 文件相关
 */
class Elf {
 public:
  /**
   * 构造函数
   * @param _kernel_image_filename 要加载的内核文件
   */
  explicit Elf(wchar_t *_kernel_image_filename);

  /**
   * 析构函数
   */
  ~Elf();

  /// @name 不使用的构造函数
  /// @{
  Elf(const Elf &) = delete;
  Elf(Elf &&) = delete;
  auto operator=(const Elf &) -> Elf & = delete;
  auto operator=(Elf &&) -> Elf & = delete;
  /// @}

  /**
   * 加载 elf 内核
   * @return 成功返回内核入口地址，失败返回 0
   */
  [[nodiscard]] auto load_kernel_image() const -> uint64_t;

  /**
   * 将 elf 文件加载进内存
   * @return 成功返回内核入口地址，失败返回 0
   */
  [[nodiscard]] auto load() const -> uintptr_t;

 private:
  /// @name elf 文件相关
  /// @{
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *file_system_protocol_ = nullptr;
  EFI_FILE *root_file_system_ = nullptr;
  /// elf 文件指针
  EFI_FILE *elf_ = nullptr;
  /// elf 文件大小
  size_t elf_file_size_ = 0;
  /// elf 文件内容缓冲区
  void *elf_file_buffer_ = nullptr;
  /// elf 文件访问
  std::span<uint8_t> file_ = {};
  Elf64_Ehdr ehdr_ = {};
  std::span<Elf64_Phdr> phdr_ = {};
  std::span<Elf64_Shdr> shdr_ = {};
  /// section 缓冲区大小
  static constexpr const size_t SECTION_BUF_SIZE = 1024;
  /// shstrtab 缓冲
  std::array<uint8_t, SECTION_BUF_SIZE> shstrtab_buf_ = {};
  /// @}

  /**
   * 获取文件大小
   * @return 文件大小
   */
  [[nodiscard]] auto get_file_size() const -> size_t;

  /**
   * 检查 elf 标识
   * @return 失败返回 false
   */
  [[nodiscard]] auto check_elf_identity() const -> bool;

  /**
   * 读取 elf 文件的 ehdr
   */
  void get_ehdr();

  /**
   * 输出 elf ehdr
   */
  void print_ehdr() const;

  /**
   * 读取 elf 文件的 phdr
   */
  void get_phdr();

  /**
   * 输出 phdr
   */
  void print_phdr() const;

  /**
   * 读取 elf 文件的 shdr
   */
  void get_shdr();

  /**
   * 输出 shdr
   */
  void print_shdr() const;

  /**
   * 将 elf 段加载到内存
   * @param _phdr 要加载的程序段 phdr
   */
  [[nodiscard]] bool load_sections(const Elf64_Phdr &_phdr) const;

  /**
   * 加载程序段
   * @return 失败返回 false
   */
  [[nodiscard]] bool load_program_sections() const;
};

#endif /* SIMPLEKERNEL_SRC_BOOT_INCLUDE_LOAD_ELF_H_ */
