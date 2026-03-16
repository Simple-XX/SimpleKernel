/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace virtio::blk {

/**
 * @brief 块设备特性位定义
 * @see virtio-v1.2#5.2.3 Feature bits
 *
 * 特性位用于在设备初始化期间协商设备功能。
 * 驱动程序通过读取设备特性位来确定设备支持哪些功能，
 * 并通过写入驱动程序特性位来确认要使用的功能。
 */
enum class BlkFeatureBit : uint64_t {
  /// 设备配置空间中 size_max 字段有效 (VIRTIO_BLK_F_SIZE_MAX)
  kSizeMax = 1ULL << 1,
  /// 设备配置空间中 seg_max 字段有效 (VIRTIO_BLK_F_SEG_MAX)
  kSegMax = 1ULL << 2,
  /// 设备配置空间中 geometry 字段有效 (VIRTIO_BLK_F_GEOMETRY)
  kGeometry = 1ULL << 4,
  /// 设备为只读设备 (VIRTIO_BLK_F_RO)
  kRo = 1ULL << 5,
  /// 设备配置空间中 blk_size 字段有效 (VIRTIO_BLK_F_BLK_SIZE)
  kBlkSize = 1ULL << 6,
  /// 设备支持缓存刷新命令 (VIRTIO_BLK_F_FLUSH)
  kFlush = 1ULL << 9,
  /// 设备配置空间中 topology 字段有效 (VIRTIO_BLK_F_TOPOLOGY)
  kTopology = 1ULL << 10,
  /// 设备可在回写和直写缓存模式间切换 (VIRTIO_BLK_F_CONFIG_WCE)
  kConfigWce = 1ULL << 11,
  /// 设备支持多队列 (VIRTIO_BLK_F_MQ)
  kMq = 1ULL << 12,
  /// 设备支持 discard 命令 (VIRTIO_BLK_F_DISCARD)
  kDiscard = 1ULL << 13,
  /// 设备支持 write zeroes 命令 (VIRTIO_BLK_F_WRITE_ZEROES)
  kWriteZeroes = 1ULL << 14,
  /// 设备支持提供存储生命周期信息 (VIRTIO_BLK_F_LIFETIME)
  kLifetime = 1ULL << 15,
  /// 设备支持 secure erase 命令 (VIRTIO_BLK_F_SECURE_ERASE)
  kSecureErase = 1ULL << 16,
};

/**
 * @brief 块设备配置空间布局
 * @see virtio-v1.2#5.2.4 Device configuration layout
 *
 * 设备配置空间包含设备的静态配置信息，如容量、最大段大小、
 * 几何信息、拓扑信息等。驱动程序通过传输层读取这些信息。
 *
 * @note 配置空间使用小端格式
 * @note 多字节字段需要使用 generation counter 机制确保读取一致性
 */
struct [[gnu::packed]] BlkConfig {
  /// 设备容量（以 512 字节扇区为单位）
  uint64_t capacity;
  /// 任意单个段的最大字节数（如果 VIRTIO_BLK_F_SIZE_MAX 被协商）
  uint32_t size_max;
  /// 单个请求中的最大段数（如果 VIRTIO_BLK_F_SEG_MAX 被协商）
  uint32_t seg_max;

  /// 磁盘几何信息（如果 VIRTIO_BLK_F_GEOMETRY 被协商）
  struct [[gnu::packed]] {
    /// 柱面数
    uint16_t cylinders;
    /// 磁头数
    uint8_t heads;
    /// 每磁道扇区数
    uint8_t sectors;
  } geometry;

  /// 块大小（字节），用于性能优化（如果 VIRTIO_BLK_F_BLK_SIZE 被协商）
  uint32_t blk_size;

  /// I/O 拓扑信息（如果 VIRTIO_BLK_F_TOPOLOGY 被协商）
  struct [[gnu::packed]] {
    /// 每个物理块包含的逻辑块数 (log2)
    uint8_t physical_block_exp;
    /// 第一个对齐逻辑块的偏移
    uint8_t alignment_offset;
    /// 建议的最小 I/O 大小（块数）
    uint16_t min_io_size;
    /// 建议的最优 I/O 大小（块数）
    uint32_t opt_io_size;
  } topology;

  /// 缓存模式：0=直写(writethrough)，1=回写(writeback)
  /// （如果 VIRTIO_BLK_F_CONFIG_WCE 被协商）
  uint8_t writeback;
  /// 保留字段，用于填充对齐
  uint8_t unused0[3];

  /// discard 命令的最大扇区数（如果 VIRTIO_BLK_F_DISCARD 被协商）
  uint32_t max_discard_sectors;
  /// discard 命令的最大段数（如果 VIRTIO_BLK_F_DISCARD 被协商）
  uint32_t max_discard_seg;
  /// discard 扇区对齐要求（如果 VIRTIO_BLK_F_DISCARD 被协商）
  uint32_t discard_sector_alignment;

  /// write zeroes 命令的最大扇区数（如果 VIRTIO_BLK_F_WRITE_ZEROES 被协商）
  uint32_t max_write_zeroes_sectors;
  /// write zeroes 命令的最大段数（如果 VIRTIO_BLK_F_WRITE_ZEROES 被协商）
  uint32_t max_write_zeroes_seg;
  /// write zeroes 是否可能导致 unmap（如果 VIRTIO_BLK_F_WRITE_ZEROES 被协商）
  uint8_t write_zeroes_may_unmap;
  /// 保留字段，用于填充对齐
  uint8_t unused1[3];

  /// secure erase 命令的最大扇区数（如果 VIRTIO_BLK_F_SECURE_ERASE 被协商）
  uint32_t max_secure_erase_sectors;
  /// secure erase 命令的最大段数（如果 VIRTIO_BLK_F_SECURE_ERASE 被协商）
  uint32_t max_secure_erase_seg;
  /// secure erase 扇区对齐要求（如果 VIRTIO_BLK_F_SECURE_ERASE 被协商）
  uint32_t secure_erase_sector_alignment;

  /// 请求队列数（如果 VIRTIO_BLK_F_MQ 被协商）
  uint16_t num_queues;
  /// 保留字段，用于未来扩展
  uint8_t unused2[6];
};

/**
 * @brief 块设备配置空间字段偏移量
 * @see virtio-v1.2#5.2.4
 *
 * 这些常量定义了各个配置字段在配置空间中的字节偏移量，
 * 用于通过传输层 ReadConfigU* 系列函数访问配置空间。
 */
enum class BlkConfigOffset : uint32_t {
  kCapacity = 0,
  kSizeMax = 8,
  kSegMax = 12,
  kGeometryCylinders = 16,
  kGeometryHeads = 18,
  kGeometrySectors = 19,
  kBlkSize = 20,
  kTopologyPhysBlockExp = 24,
  kTopologyAlignOffset = 25,
  kTopologyMinIoSize = 26,
  kTopologyOptIoSize = 28,
  kWriteback = 32,
  kMaxDiscardSectors = 36,
  kMaxDiscardSeg = 40,
  kDiscardSectorAlignment = 44,
  kMaxWriteZeroesSectors = 48,
  kMaxWriteZeroesSeg = 52,
  kWriteZeroesMayUnmap = 56,
  kMaxSecureEraseSectors = 60,
  kMaxSecureEraseSeg = 64,
  kSecureEraseSectorAlignment = 68,
  kNumQueues = 72,
};

/**
 * @brief 块设备请求类型
 * @see virtio-v1.2#5.2.6 Device Operation
 *
 * 定义了块设备支持的各种请求操作类型。
 * 请求类型存储在请求头的 type 字段中。
 */
enum class ReqType : uint32_t {
  /// 读取 (VIRTIO_BLK_T_IN)
  kIn = 0,
  /// 写入 (VIRTIO_BLK_T_OUT)
  kOut = 1,
  /// 刷新缓存 (VIRTIO_BLK_T_FLUSH)
  kFlush = 4,
  /// 获取设备 ID (VIRTIO_BLK_T_GET_ID)
  kGetId = 8,
  /// 获取设备生命周期信息 (VIRTIO_BLK_T_GET_LIFETIME)
  kGetLifetime = 10,
  /// 丢弃扇区 (VIRTIO_BLK_T_DISCARD)
  kDiscard = 11,
  /// 写零 (VIRTIO_BLK_T_WRITE_ZEROES)
  kWriteZeroes = 13,
  /// 安全擦除 (VIRTIO_BLK_T_SECURE_ERASE)
  kSecureErase = 14,
};

/**
 * @brief 块设备请求状态
 * @see virtio-v1.2#5.2.6
 *
 * 设备在请求完成后，在响应中写入状态字节。
 */
enum class BlkStatus : uint8_t {
  /// 操作成功 (VIRTIO_BLK_S_OK)
  kOk = 0,
  /// IO 错误 (VIRTIO_BLK_S_IOERR)
  kIoErr = 1,
  /// 不支持的操作 (VIRTIO_BLK_S_UNSUPP)
  kUnsupp = 2,
};

/**
 * @brief 块设备请求头
 * @see virtio-v1.2#5.2.6 Device Operation
 *
 * 所有块设备请求都以此结构开头，位于第一个描述符中（设备只读）。
 *
 * @note 协议中所有字段采用小端格式
 * @note 请求头后跟数据缓冲区（可选），最后是状态字节（设备只写）
 */
struct [[gnu::packed]] BlkReqHeader {
  /// 请求类型 (ReqType)
  uint32_t type;
  /// 保留字段，必须为 0
  uint32_t reserved;
  /// 起始扇区号（仅对读/写请求有效，其他类型应设为 0）
  uint64_t sector;
};

/**
 * @brief Discard/Write Zeroes/Secure Erase 请求段
 * @see virtio-v1.2#5.2.6
 *
 * VIRTIO_BLK_T_DISCARD、VIRTIO_BLK_T_WRITE_ZEROES 和
 * VIRTIO_BLK_T_SECURE_ERASE 请求的数据部分由一个或多个此结构的实例组成。
 */
struct [[gnu::packed]] BlkDiscardWriteZeroes {
  /// 起始扇区（以 512 字节为单位）
  uint64_t sector;
  /// 扇区数（以 512 字节为单位）
  uint32_t num_sectors;
  /// 标志位
  struct [[gnu::packed]] {
    /// 对于 write zeroes: 允许设备 unmap（取消映射）指定范围
    /// 对于 discard/secure erase: 保留，必须为 0
    uint32_t unmap : 1;
    /// 保留位，必须为 0
    uint32_t reserved : 31;
  } flags;
};

/**
 * @brief 设备生命周期信息
 * @see virtio-v1.2#5.2.6
 *
 * VIRTIO_BLK_T_GET_LIFETIME 请求的响应数据。
 * 用于 eMMC/UFS 等存储设备报告磨损程度。
 */
struct [[gnu::packed]] BlkLifetime {
  /**
   * @brief Pre-EOL 信息常量
   */
  enum class PreEolInfo : uint16_t {
    /// 0: 值未定义 (VIRTIO_BLK_PRE_EOL_INFO_UNDEFINED)
    kUndefined = 0,
    /// 1: 正常，< 80% 保留块已消耗 (VIRTIO_BLK_PRE_EOL_INFO_NORMAL)
    kNormal = 1,
    /// 2: 警告，80% 保留块已消耗 (VIRTIO_BLK_PRE_EOL_INFO_WARNING)
    kWarning = 2,
    /// 3: 紧急，90% 保留块已消耗 (VIRTIO_BLK_PRE_EOL_INFO_URGENT)
    kUrgent = 3,
  };

  /// 预 EOL (End-Of-Life) 信息
  uint16_t pre_eol_info;
  /// 设备生命周期估计 A（SLC 单元磨损）
  /// 0x01-0x0a: 使用了 x*10% 生命周期
  /// 0x0b: 超出预估生命周期
  uint16_t device_lifetime_est_typ_a;
  /// 设备生命周期估计 B（MLC 单元磨损）
  /// 含义同 device_lifetime_est_typ_a
  uint16_t device_lifetime_est_typ_b;
};

/// 标准扇区大小（字节）
inline constexpr size_t kSectorSize = 512;

/// GET_ID 请求返回的设备 ID 字符串最大长度（字节）
/// @note 如果字符串长度为 20 字节，则没有 NUL 终止符
inline constexpr size_t kDeviceIdMaxLen = 20;

/**
 * @brief VirtIO 设备性能监控统计数据
 */
struct VirtioStats {
  /// 已传输字节数
  uint64_t bytes_transferred{0};
  /// 借助 Event Index 省略的 Kick 次数
  uint64_t kicks_elided{0};
  /// 已处理的中断次数
  uint64_t interrupts_handled{0};
  /// 队列满导致入队失败的次数
  uint64_t queue_full_errors{0};
};

}  // namespace virtio::blk
