/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <cpu_io.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

#include "expected.hpp"
#include "io_buffer.hpp"
#include "kernel_log.hpp"
#include "virtio/defs.h"
#include "virtio/device/blk/virtio_blk_defs.h"
#include "virtio/device/device_initializer.hpp"
#include "virtio/transport/mmio.hpp"
#include "virtio/virt_queue/split.hpp"

namespace virtio::blk {

/**
 * @brief Virtio 块设备驱动
 *
 * virtio 块设备是一个简单的虚拟块设备（即磁盘）。
 * 读写请求（以及其他特殊请求）被放置在请求队列中，由设备服务（可能乱序）。
 *
 * 该类封装了 VirtIO 块设备的完整生命周期：
 * - 传输层的创建和管理（通过 TransportT 模板参数泛化）
 * - Virtqueue 的创建和管理（通过 VirtqueueT 模板参数泛化）
 * - 设备初始化序列（特性协商、队列配置、设备激活）
 * - 异步 IO 接口（Enqueue/Kick/HandleInterrupt 回调模型）
 * - 同步读写便捷方法（基于异步接口实现）
 *
 * 用户只需提供 MMIO 基地址和 DMA 缓冲区，
 * 即可通过 Read() / Write() 或异步接口进行块设备操作。
 *
 * @tparam TransportT 传输层类型（默认 MmioTransport）
 * @tparam VirtqueueT Virtqueue 类型（默认 SplitVirtqueue）
 * @tparam VirtqueueT Virtqueue 模板（默认 SplitVirtqueue）
 * @see virtio-v1.2#5.2 Block Device
 * @see 架构文档 §3
 */
template <typename TransportT = MmioTransport,
          typename VirtqueueT = SplitVirtqueue>
class VirtioBlk {
 public:
  /// 异步 IO 回调中使用的用户自定义上下文指针类型
  using UserData = void*;

  /// 每个设备的最大并发(in-flight)请求数
  static constexpr uint16_t kMaxInflight = 64;

  /// 每个 Scatter-Gather 请求的最大 IoVec 数量（含请求头和状态字节）
  static constexpr size_t kMaxSgElements = 18;

  /**
   * @brief 获取多队列所需的总 DMA 内存大小
   *
   * 调用者应根据此值预分配页对齐、已清零的 DMA 内存。
   *
   * @param queue_count 请求的队列数量
   * @param queue_size 每个队列的描述符数量（必须为 2 的幂）
   * @return pair.first = 总字节数，pair.second = 对齐要求（字节）
   * @see 架构文档 §3
   */
  [[nodiscard]] static constexpr auto GetRequiredVqMemSize(uint16_t queue_count,
                                                           uint32_t queue_size)
      -> std::pair<size_t, size_t> {
    // 始终按 event_idx=true 分配，因为特性协商在分配之后
    size_t per_queue =
        VirtqueueT::CalcSize(static_cast<uint16_t>(queue_size), true);
    return {per_queue * queue_count, 4096};
  }

  /**
   * @brief 计算单个 Virtqueue DMA 缓冲区所需的字节数（向后兼容）
   *
   * @param queue_size 队列大小（2 的幂，默认 128）
   * @return 所需的 DMA 内存字节数
   */
  [[nodiscard]] static constexpr auto CalcDmaSize(uint16_t queue_size = 128)
      -> size_t {
    // 始终按 event_idx=true 分配，确保空间充足
    return VirtqueueT::CalcSize(queue_size, true);
  }

  /**
   * @brief 计算 RequestSlot DMA 内存所需字节数
   *
   * @return pair.first = 总字节数，pair.second = 对齐要求（字节）
   */
  [[nodiscard]] static constexpr auto GetRequiredSlotMemSize()
      -> std::pair<size_t, size_t> {
    return {sizeof(RequestSlot) * kMaxInflight, alignof(RequestSlot)};
  }

  /**
   * @brief 创建并初始化块设备
   *
   * 内部自动完成：
   * 1. Transport 初始化和验证
   * 2. Virtqueue 创建
   * 3. VirtIO 设备初始化序列（重置、特性协商、队列配置、设备激活）
   *
   * @param mmio_base MMIO 设备基地址
   * @param vq_dma 预分配的 Virtqueue DMA 内存区域
   *        （页对齐，已清零，大小 >= GetRequiredVqMemSize()）
   * @param slot_dma 预分配的 RequestSlot DMA 内存区域
   *        （大小 >= GetRequiredSlotMemSize()）
   * @param virt_to_phys 虚拟地址到物理地址转换函数（默认恒等映射）
   * @param queue_count 期望的队列数量（当前仅支持 1）
   * @param queue_size 每个队列的描述符数量（2 的幂，默认 128）
   * @param driver_features 额外的驱动特性位（VERSION_1 自动包含）
   * @return 成功返回 VirtioBlk 实例，失败返回错误
   * @see virtio-v1.2#3.1.1 Driver Requirements: Device Initialization
   */
  [[nodiscard]] static auto Create(
      uint64_t mmio_base, const DmaRegion& vq_dma, const DmaRegion& slot_dma,
      VirtToPhysFunc virt_to_phys = IdentityVirtToPhys,
      uint16_t queue_count = 1, uint32_t queue_size = 128,
      uint64_t driver_features = 0) -> Expected<VirtioBlk> {
    if (queue_count == 0) {
      return std::unexpected(Error{ErrorCode::kInvalidArgument});
    }
    if (queue_count > 1) {
    }

    // 1. 创建传输层
    TransportT transport(mmio_base);
    if (!transport.IsValid()) {
      return std::unexpected(Error{ErrorCode::kTransportNotInitialized});
    }

    // 2. 设备初始化序列
    DeviceInitializer<TransportT> initializer(transport);

    uint64_t wanted_features =
        static_cast<uint64_t>(ReservedFeature::kVersion1) |
        static_cast<uint64_t>(ReservedFeature::kEventIdx) | driver_features;
    auto negotiated_result = initializer.Init(wanted_features);
    if (!negotiated_result) {
      return std::unexpected(negotiated_result.error());
    }
    uint64_t negotiated = *negotiated_result;

    if ((negotiated & static_cast<uint64_t>(ReservedFeature::kVersion1)) == 0) {
      return std::unexpected(Error{ErrorCode::kFeatureNegotiationFailed});
    }

    // 根据协商结果决定是否启用 Event Index
    bool event_idx =
        (negotiated & static_cast<uint64_t>(ReservedFeature::kEventIdx)) != 0;
    if (event_idx) {
    }
    // 3. 创建 Virtqueue
    VirtqueueT vq(vq_dma, static_cast<uint16_t>(queue_size), event_idx);
    if (!vq.IsValid()) {
      return std::unexpected(Error{ErrorCode::kInvalidArgument});
    }

    // 4. 配置队列
    const uint32_t queue_idx = 0;
    auto setup_result = initializer.SetupQueue(
        queue_idx, vq.DescPhys(), vq.AvailPhys(), vq.UsedPhys(), vq.Size());
    if (!setup_result) {
      return std::unexpected(setup_result.error());
    }

    // 5. 激活设备
    auto activate_result = initializer.Activate();
    if (!activate_result) {
      return std::unexpected(activate_result.error());
    }

    return VirtioBlk(std::move(transport), std::move(vq), negotiated, slot_dma,
                     virt_to_phys);
  }

  // ======== 异步 IO 接口 (Enqueue/Kick/HandleInterrupt) ========

  /**
   * @brief 异步提交读请求（仅入队描述符，不触发硬件通知）
   *
   * 构建 virtio-blk 请求描述符链（header + data buffers + status），
   * 提交到 Available Ring，但不通知设备。调用者需随后调用 Kick() 通知。
   *
   * @param queue_index 队列索引（当前仅支持 0）
   * @param sector 起始扇区号（以 512 字节为单位）
   * @param buffers 数据缓冲区 IoVec 数组（物理地址 + 长度）
   * @param buffer_count buffers 数组中的元素数量
   * @param token 用户自定义上下文指针，在 HandleInterrupt 回调时原样传回
   * @return 成功或失败
   * @see virtio-v1.2#5.2.6 Device Operation
   */
  [[nodiscard]] auto EnqueueRead(uint16_t queue_index, uint64_t sector,
                                 const IoVec* buffers, size_t buffer_count,
                                 UserData token = nullptr) -> Expected<void> {
    return DoEnqueue(ReqType::kIn, queue_index, sector, buffers, buffer_count,
                     token);
  }

  /**
   * @brief 异步提交写请求（仅入队描述符，不触发硬件通知）
   *
   * 构建 virtio-blk 请求描述符链（header + data buffers + status），
   * 提交到 Available Ring，但不通知设备。调用者需随后调用 Kick() 通知。
   *
   * 针对 Write 操作，数据缓冲区的描述符 flag 为设备只读（无
   * VRING_DESC_F_WRITE）。
   *
   * @param queue_index 队列索引（当前仅支持 0）
   * @param sector 起始扇区号（以 512 字节为单位）
   * @param buffers 数据缓冲区 IoVec 数组（物理地址 + 长度）
   * @param buffer_count buffers 数组中的元素数量
   * @param token 用户自定义上下文指针，在 HandleInterrupt 回调时原样传回
   * @return 成功或失败
   * @see virtio-v1.2#5.2.6 Device Operation
   */
  [[nodiscard]] auto EnqueueWrite(uint16_t queue_index, uint64_t sector,
                                  const IoVec* buffers, size_t buffer_count,
                                  UserData token = nullptr) -> Expected<void> {
    return DoEnqueue(ReqType::kOut, queue_index, sector, buffers, buffer_count,
                     token);
  }

  /**
   * @brief 批量触发硬件通知
   *
   * 通知设备 Available Ring 中有新的待处理请求。
   * 调用者应在 EnqueueRead/EnqueueWrite 后调用此方法。
   *
   * @param queue_index 队列索引（当前仅支持 0）
   * @see virtio-v1.2#2.7.13 Supplying Buffers to The Device
   */
  auto Kick(uint16_t queue_index) -> void {
    if (queue_index != 0) {
      return;
    }
    // 写屏障：确保 Available Ring 更新对设备可见
    cpu_io::Wmb();

    if (vq_.EventIdxEnabled()) {
      auto* avail_event_ptr = vq_.UsedAvailEvent();
      if (avail_event_ptr != nullptr) {
        uint16_t avail_event = *avail_event_ptr;
        uint16_t new_idx = vq_.AvailIdx();
        if (VringNeedEvent(avail_event, new_idx, old_avail_idx_)) {
          transport_.NotifyQueue(queue_index);
        } else {
          stats_.kicks_elided++;
        }
        old_avail_idx_ = new_idx;
      } else {
        transport_.NotifyQueue(queue_index);
      }
    } else {
      transport_.NotifyQueue(queue_index);
    }
  }

  /**
   * @brief 中断处理（带完成回调）
   *
   * 在 ISR 或轮询循环中调用。确认设备中断，遍历 Used Ring 中已完成的请求，
   * 对每个请求调用 on_complete 回调，释放描述符链和请求槽。
   *
   * @tparam CompletionCallback 签名要求：void(UserData token, ErrorCode status)
   *         - token: 提交时传入的用户上下文指针
   *         - status: 设备返回的完成状态映射为 ErrorCode
   * @param on_complete 完成回调函数
   * @see virtio-v1.2#2.7.14 Receiving Used Buffers From The Device
   */
  template <typename CompletionCallback>
  auto HandleInterrupt(CompletionCallback&& on_complete) -> void {
    // 确认中断
    uint32_t isr_status = transport_.GetInterruptStatus();
    if (isr_status != 0) {
      transport_.AckInterrupt(isr_status);
    }
    stats_.interrupts_handled++;

    ProcessCompletions(static_cast<CompletionCallback&&>(on_complete));
    UpdateUsedEvent();
  }

  /**
   * @brief 中断处理（简化版，无回调）
   *
   * 仅确认设备中断并设置完成标志，供同步轮询使用。
   * 不处理 Used Ring 和描述符回收。
   *
   * @note 此方法可在中断上下文中安全调用（ISR-safe）
   * @see virtio-v1.2#2.3 Notifications
   */
  auto HandleInterrupt() -> void {
    uint32_t status = transport_.GetInterruptStatus();
    if (status != 0) {
      transport_.AckInterrupt(status);
    }
    stats_.interrupts_handled++;
    request_completed_ = true;
    cpu_io::Wmb();
  }

  // ======== 同步便捷方法 ========

  /**
   * @brief 同步读取一个扇区
   *
   * 基于异步接口实现：EnqueueRead → Kick → 轮询 HandleInterrupt → 返回。
   *
   * @param sector 起始扇区号（以 512 字节为单位）
   * @param data 数据缓冲区（至少 kSectorSize 字节，必须位于 DMA 可访问内存）
   * @return 成功或失败
   * @note 缓冲区在函数返回前不得释放或修改
   * @see virtio-v1.2#5.2.6 Device Operation
   */
  [[nodiscard]] auto Read(uint64_t sector, uint8_t* data) -> Expected<void> {
    if (data == nullptr) {
      return std::unexpected(Error{ErrorCode::kInvalidArgument});
    }
    IoVec data_iov{virt_to_phys_(reinterpret_cast<uintptr_t>(data)),
                   kSectorSize};
    return SubmitSyncRequest(ReqType::kIn, sector, &data_iov, 1);
  }

  /**
   * @brief 同步写入一个扇区
   *
   * 基于异步接口实现：EnqueueWrite → Kick → 轮询 HandleInterrupt → 返回。
   *
   * @param sector 起始扇区号（以 512 字节为单位）
   * @param data 数据缓冲区（至少 kSectorSize 字节，必须位于 DMA 可访问内存）
   * @return 成功或失败
   * @note 如果设备协商了 VIRTIO_BLK_F_RO，写请求将返回错误
   * @see virtio-v1.2#5.2.6 Device Operation
   */
  [[nodiscard]] auto Write(uint64_t sector, const uint8_t* data)
      -> Expected<void> {
    if (data == nullptr) {
      return std::unexpected(Error{ErrorCode::kInvalidArgument});
    }
    IoVec data_iov{
        virt_to_phys_(reinterpret_cast<uintptr_t>(const_cast<uint8_t*>(data))),
        kSectorSize};
    return SubmitSyncRequest(ReqType::kOut, sector, &data_iov, 1);
  }

  // ======== 配置与监控 ========

  /**
   * @brief 读取块设备配置空间
   *
   * 读取设备的配置信息，包括容量、几何信息、拓扑信息等。
   * 配置空间的可用字段取决于协商的特性位。
   *
   * @return 块设备配置结构
   * @see virtio-v1.2#5.2.4 Device configuration layout
   */
  [[nodiscard]] auto ReadConfig() const -> BlkConfig {
    BlkConfig config{};

    config.capacity = transport_.ReadConfigU64(
        static_cast<uint32_t>(BlkConfigOffset::kCapacity));
    config.size_max = transport_.ReadConfigU32(
        static_cast<uint32_t>(BlkConfigOffset::kSizeMax));
    config.seg_max = transport_.ReadConfigU32(
        static_cast<uint32_t>(BlkConfigOffset::kSegMax));

    if ((negotiated_features_ &
         static_cast<uint64_t>(BlkFeatureBit::kGeometry)) != 0) {
      config.geometry.cylinders = transport_.ReadConfigU16(
          static_cast<uint32_t>(BlkConfigOffset::kGeometryCylinders));
      config.geometry.heads = transport_.ReadConfigU8(
          static_cast<uint32_t>(BlkConfigOffset::kGeometryHeads));
      config.geometry.sectors = transport_.ReadConfigU8(
          static_cast<uint32_t>(BlkConfigOffset::kGeometrySectors));
    }

    if ((negotiated_features_ &
         static_cast<uint64_t>(BlkFeatureBit::kBlkSize)) != 0) {
      config.blk_size = transport_.ReadConfigU32(
          static_cast<uint32_t>(BlkConfigOffset::kBlkSize));
    }

    if ((negotiated_features_ &
         static_cast<uint64_t>(BlkFeatureBit::kTopology)) != 0) {
      config.topology.physical_block_exp = transport_.ReadConfigU8(
          static_cast<uint32_t>(BlkConfigOffset::kTopologyPhysBlockExp));
      config.topology.alignment_offset = transport_.ReadConfigU8(
          static_cast<uint32_t>(BlkConfigOffset::kTopologyAlignOffset));
      config.topology.min_io_size = transport_.ReadConfigU16(
          static_cast<uint32_t>(BlkConfigOffset::kTopologyMinIoSize));
      config.topology.opt_io_size = transport_.ReadConfigU32(
          static_cast<uint32_t>(BlkConfigOffset::kTopologyOptIoSize));
    }

    if ((negotiated_features_ &
         static_cast<uint64_t>(BlkFeatureBit::kConfigWce)) != 0) {
      config.writeback = transport_.ReadConfigU8(
          static_cast<uint32_t>(BlkConfigOffset::kWriteback));
    }

    if ((negotiated_features_ &
         static_cast<uint64_t>(BlkFeatureBit::kDiscard)) != 0) {
      config.max_discard_sectors = transport_.ReadConfigU32(
          static_cast<uint32_t>(BlkConfigOffset::kMaxDiscardSectors));
      config.max_discard_seg = transport_.ReadConfigU32(
          static_cast<uint32_t>(BlkConfigOffset::kMaxDiscardSeg));
      config.discard_sector_alignment = transport_.ReadConfigU32(
          static_cast<uint32_t>(BlkConfigOffset::kDiscardSectorAlignment));
    }

    if ((negotiated_features_ &
         static_cast<uint64_t>(BlkFeatureBit::kWriteZeroes)) != 0) {
      config.max_write_zeroes_sectors = transport_.ReadConfigU32(
          static_cast<uint32_t>(BlkConfigOffset::kMaxWriteZeroesSectors));
      config.max_write_zeroes_seg = transport_.ReadConfigU32(
          static_cast<uint32_t>(BlkConfigOffset::kMaxWriteZeroesSeg));
      config.write_zeroes_may_unmap = transport_.ReadConfigU8(
          static_cast<uint32_t>(BlkConfigOffset::kWriteZeroesMayUnmap));
    }

    if ((negotiated_features_ &
         static_cast<uint64_t>(BlkFeatureBit::kSecureErase)) != 0) {
      config.max_secure_erase_sectors = transport_.ReadConfigU32(
          static_cast<uint32_t>(BlkConfigOffset::kMaxSecureEraseSectors));
      config.max_secure_erase_seg = transport_.ReadConfigU32(
          static_cast<uint32_t>(BlkConfigOffset::kMaxSecureEraseSeg));
      config.secure_erase_sector_alignment = transport_.ReadConfigU32(
          static_cast<uint32_t>(BlkConfigOffset::kSecureEraseSectorAlignment));
    }

    if ((negotiated_features_ & static_cast<uint64_t>(BlkFeatureBit::kMq)) !=
        0) {
      config.num_queues = transport_.ReadConfigU16(
          static_cast<uint32_t>(BlkConfigOffset::kNumQueues));
    }

    return config;
  }

  /**
   * @brief 获取设备容量
   *
   * @return 设备容量（以 512 字节扇区为单位）
   * @see virtio-v1.2#5.2.4
   */
  [[nodiscard]] auto GetCapacity() const -> uint64_t {
    return transport_.ReadConfigU64(
        static_cast<uint32_t>(BlkConfigOffset::kCapacity));
  }

  /**
   * @brief 获取协商后的特性位
   *
   * @return 设备和驱动程序都支持的特性位掩码
   */
  [[nodiscard]] auto GetNegotiatedFeatures() const -> uint64_t {
    return negotiated_features_;
  }

  /**
   * @brief 获取性能监控统计数据
   *
   * @return 当前统计数据的快照
   * @see 架构文档 §3
   */
  [[nodiscard]] auto GetStats() const -> VirtioStats { return stats_; }

  /// @name 构造/析构函数
  /// @{
  VirtioBlk() = delete;
  VirtioBlk(VirtioBlk&& other) noexcept
      : transport_(std::move(other.transport_)),
        vq_(std::move(other.vq_)),
        negotiated_features_(other.negotiated_features_),
        slot_dma_(other.slot_dma_),
        slots_(other.slots_),
        virt_to_phys_(other.virt_to_phys_),
        stats_(other.stats_),
        slot_bitmap_(other.slot_bitmap_),
        old_avail_idx_(other.old_avail_idx_),
        request_completed_(other.request_completed_) {
    other.slots_ = nullptr;
    other.slot_bitmap_ = 0;
  }
  auto operator=(VirtioBlk&& other) noexcept -> VirtioBlk& {
    if (this != &other) {
      transport_ = std::move(other.transport_);
      vq_ = std::move(other.vq_);
      negotiated_features_ = other.negotiated_features_;
      slot_dma_ = other.slot_dma_;
      slots_ = other.slots_;
      virt_to_phys_ = other.virt_to_phys_;
      stats_ = other.stats_;
      slot_bitmap_ = other.slot_bitmap_;
      old_avail_idx_ = other.old_avail_idx_;
      request_completed_ = other.request_completed_;
      other.slots_ = nullptr;
      other.slot_bitmap_ = 0;
    }
    return *this;
  }
  VirtioBlk(const VirtioBlk&) = delete;
  auto operator=(const VirtioBlk&) -> VirtioBlk& = delete;
  ~VirtioBlk() = default;
  /// @}

 private:
  /**
   * @brief 异步请求上下文槽
   *
   * 每个 in-flight 请求占用一个槽，存储请求头（DMA可访问）、
   * 状态字节（设备回写）、用户 token 和描述符链头索引。
   * 槽的占用状态由 slot_bitmap_ 管理。
   */
  struct RequestSlot {
    /// 请求头（DMA 可访问，设备只读）
    alignas(16) BlkReqHeader header;
    /// 状态字节（DMA 可访问，设备只写）
    alignas(4) volatile uint8_t status;
    /// 用户自定义上下文指针
    UserData token;
    /// 描述符链头索引（用于在 Used Ring 中匹配）
    uint16_t desc_head;
  };

  /**
   * @brief 私有构造函数
   *
   * 只能通过 Create() 静态工厂方法创建实例。
   */
  VirtioBlk(TransportT transport, VirtqueueT vq, uint64_t features,
            const DmaRegion& slot_dma, VirtToPhysFunc v2p)
      : transport_(std::move(transport)),
        vq_(std::move(vq)),
        negotiated_features_(features),
        slot_dma_(slot_dma),
        slots_(reinterpret_cast<RequestSlot*>(slot_dma.Data())),
        virt_to_phys_(v2p),
        stats_{},
        slot_bitmap_(0),
        old_avail_idx_(0),
        request_completed_(false) {}

  /**
   * @brief 异步入队请求的内部实现
   *
   * 分配请求槽，填充请求头，构建 Scatter-Gather 描述符链，提交到 Available
   * Ring。
   *
   * @param type 请求类型（kIn/kOut）
   * @param queue_index 队列索引
   * @param sector 起始扇区号
   * @param buffers 数据缓冲区 IoVec 数组
   * @param buffer_count 缓冲区数量
   * @param token 用户上下文指针
   * @return 成功或失败
   */
  [[nodiscard]] auto DoEnqueue(ReqType type, uint16_t queue_index,
                               uint64_t sector, const IoVec* buffers,
                               size_t buffer_count, UserData token)
      -> Expected<void> {
    if (queue_index != 0) {
      return std::unexpected(Error{ErrorCode::kInvalidArgument});
    }

    if (buffer_count + 2 > kMaxSgElements) {
      return std::unexpected(Error{ErrorCode::kInvalidArgument});
    }

    auto slot_result = AllocRequestSlot();
    if (!slot_result) {
      stats_.queue_full_errors++;
      return std::unexpected(slot_result.error());
    }
    uint16_t slot_idx = *slot_result;
    auto& slot = slots_[slot_idx];

    slot.header.type = static_cast<uint32_t>(type);
    slot.header.reserved = 0;
    slot.header.sector = sector;
    slot.status = 0xFF;  // sentinel：设备完成后会覆写
    slot.token = token;

    std::array<IoVec, kMaxSgElements> readable_iovs{};
    std::array<IoVec, kMaxSgElements> writable_iovs{};
    size_t readable_count = 0;
    size_t writable_count = 0;

    auto slot_base_phys =
        slot_dma_.phys + static_cast<size_t>(slot_idx) * sizeof(RequestSlot);
    readable_iovs[readable_count++] = {
        slot_base_phys + offsetof(RequestSlot, header), sizeof(BlkReqHeader)};

    if (type == ReqType::kIn) {
      for (size_t i = 0; i < buffer_count; ++i) {
        writable_iovs[writable_count++] = buffers[i];
      }
    } else {
      for (size_t i = 0; i < buffer_count; ++i) {
        readable_iovs[readable_count++] = buffers[i];
      }
    }

    // 状态字节始终为 device-writable
    writable_iovs[writable_count++] = {
        slot_base_phys + offsetof(RequestSlot, status), sizeof(uint8_t)};

    cpu_io::Wmb();

    auto chain_result = vq_.SubmitChain(readable_iovs.data(), readable_count,
                                        writable_iovs.data(), writable_count);
    if (!chain_result) {
      FreeRequestSlot(slot_idx);
      stats_.queue_full_errors++;
      return std::unexpected(chain_result.error());
    }

    slot.desc_head = *chain_result;

    return {};
  }

  /**
   * @brief 处理 Used Ring 中已完成的请求
   *
   * 遍历 Used Ring，对每个已完成的请求：
   * 1. 查找对应的请求槽
   * 2. 读取设备返回的状态字节
   * 3. 调用回调函数
   * 4. 释放描述符链和请求槽
   *
   * @tparam CompletionCallback void(UserData token, ErrorCode status)
   * @param on_complete 完成回调
   */
  template <typename CompletionCallback>
  auto ProcessCompletions(CompletionCallback&& on_complete) -> void {
    cpu_io::Rmb();

    while (vq_.HasUsed()) {
      auto elem_result = vq_.PopUsed();
      if (!elem_result) {
        break;
      }

      auto elem = *elem_result;
      auto head = static_cast<uint16_t>(elem.id);

      uint16_t slot_idx = FindSlotByDescHead(head);
      if (slot_idx < kMaxInflight) {
        auto& slot = slots_[slot_idx];

        cpu_io::Rmb();

        ErrorCode ec = MapBlkStatus(slot.status);
        on_complete(slot.token, ec);
        stats_.bytes_transferred += elem.len;
        FreeRequestSlot(slot_idx);
      }

      (void)vq_.FreeChain(head);
    }
  }

  /**
   * @brief 从请求槽池中分配一个空闲槽（O(1) 位图算法）
   *
   * 使用 __builtin_ctzll 找到 slot_bitmap_ 中最低的 0 位。
   *
   * @return 成功返回槽索引，失败返回错误
   */
  [[nodiscard]] auto AllocRequestSlot() -> Expected<uint16_t> {
    uint64_t free_bits = ~slot_bitmap_;
    if (free_bits == 0) {
      return std::unexpected(Error{ErrorCode::kNoFreeDescriptors});
    }
    auto idx = static_cast<uint16_t>(__builtin_ctzll(free_bits));
    if (idx >= kMaxInflight) {
      return std::unexpected(Error{ErrorCode::kNoFreeDescriptors});
    }
    slot_bitmap_ |= (uint64_t{1} << idx);
    return idx;
  }

  /**
   * @brief 释放请求槽
   *
   * @param idx 槽索引
   */
  auto FreeRequestSlot(uint16_t idx) -> void {
    if (idx < kMaxInflight) {
      slot_bitmap_ &= ~(uint64_t{1} << idx);
    }
  }

  /**
   * @brief 根据描述符链头索引查找请求槽
   *
   * @param desc_head 描述符链头索引
   * @return 匹配的槽索引，未找到则返回 kMaxInflight
   */
  [[nodiscard]] auto FindSlotByDescHead(uint16_t desc_head) const -> uint16_t {
    uint64_t used = slot_bitmap_;
    while (used != 0) {
      auto i = static_cast<uint16_t>(__builtin_ctzll(used));
      if (slots_[i].desc_head == desc_head) {
        return i;
      }
      used &= used - 1;  // clear lowest set bit
    }
    return kMaxInflight;
  }

  /**
   * @brief 将设备 BlkStatus 映射为 ErrorCode
   *
   * @param status 设备返回的原始状态字节
   * @return 对应的 ErrorCode
   */
  [[nodiscard]] static auto MapBlkStatus(uint8_t status) -> ErrorCode {
    switch (status) {
      case static_cast<uint8_t>(BlkStatus::kOk):
        return ErrorCode::kSuccess;
      case static_cast<uint8_t>(BlkStatus::kIoErr):
        return ErrorCode::kIoError;
      case static_cast<uint8_t>(BlkStatus::kUnsupp):
        return ErrorCode::kNotSupported;
      default:
        return ErrorCode::kDeviceError;
    }
  }

  /**
   * @brief 判断是否需要发送通知（处理 wrap-around）
   *
   * 基于 virtio 规范中的 vring_need_event 算法：检查 event_idx
   * 是否落在 (old, new] 区间内（含 uint16_t 回绕处理）。
   *
   * @param event_idx 设备/驱动期望的通知阈值
   * @param new_idx 当前索引
   * @param old_idx 上次通知时的索引
   * @return true 表示需要发送通知
   * @see virtio-v1.2#2.7.10 Available Buffer Notification Suppression
   */
  [[nodiscard]] static auto VringNeedEvent(uint16_t event_idx, uint16_t new_idx,
                                           uint16_t old_idx) -> bool {
    return static_cast<uint16_t>(new_idx - event_idx - 1) <
           static_cast<uint16_t>(new_idx - old_idx);
  }

  /**
   * @brief 更新 avail->used_event 字段
   *
   * 在处理完 Used Ring 后调用，告知设备下次在此索引之后再发送中断。
   * 仅在协商了 VIRTIO_F_EVENT_IDX 时生效。
   *
   * @see virtio-v1.2#2.7.10 Available Buffer Notification Suppression
   */
  auto UpdateUsedEvent() -> void {
    if (vq_.EventIdxEnabled()) {
      auto* used_event_ptr = vq_.AvailUsedEvent();
      if (used_event_ptr != nullptr) {
        *used_event_ptr = vq_.LastUsedIdx();
        cpu_io::Wmb();
      }
    }
  }

  /**
   * @brief 同步提交请求的内部实现
   *
   * Read()/Write() 的共享实现：入队 → Kick → 轮询等待 → 处理完成 → 返回。
   * 轮询上限固定为 100000000 次迭代。
   *
   * @param type 请求类型（kIn/kOut）
   * @param sector 起始扇区号
   * @param buffers 数据缓冲区 IoVec 数组
   * @param buffer_count 缓冲区数量
   * @return 成功或失败
   */
  [[nodiscard]] auto SubmitSyncRequest(ReqType type, uint64_t sector,
                                       const IoVec* buffers,
                                       size_t buffer_count) -> Expected<void> {
    auto enq = DoEnqueue(type, 0, sector, buffers, buffer_count, nullptr);
    if (!enq) {
      return std::unexpected(enq.error());
    }

    Kick(0);

    uint32_t spin_limit = 100000000U;

    for (uint32_t i = 0; i < spin_limit; ++i) {
      cpu_io::Rmb();
      if (vq_.HasUsed()) {
        break;
      }
    }

    if (!vq_.HasUsed()) {
      klog::Warn("Sync request timeout: sector={}", sector);
      return std::unexpected(Error{ErrorCode::kTimeout});
    }

    ErrorCode result = ErrorCode::kSuccess;
    bool done = false;
    ProcessCompletions([&done, &result](UserData, ErrorCode status) {
      done = true;
      result = status;
    });
    UpdateUsedEvent();

    if (!done) {
      return std::unexpected(Error{ErrorCode::kTimeout});
    }
    if (result != ErrorCode::kSuccess) {
      return std::unexpected(Error{result});
    }
    return {};
  }
  /// 传输层实例
  TransportT transport_;
  /// Virtqueue 实例（当前支持单队列）
  VirtqueueT vq_;
  /// 协商后的特性位掩码
  uint64_t negotiated_features_{0};
  /// DMA region backing the request slot pool
  DmaRegion slot_dma_{};
  /// Pointer to request slot array (lives in slot_dma_ memory)
  RequestSlot* slots_{nullptr};
  /// Address translation callback
  VirtToPhysFunc virt_to_phys_{IdentityVirtToPhys};
  /// 性能统计数据
  VirtioStats stats_;
  /// 请求槽占用位图（bit i = 1 表示 slots_[i] 被占用）
  uint64_t slot_bitmap_{};
  /// 上次 Kick 时的 avail idx（用于 Event Index 通知抑制）
  uint16_t old_avail_idx_{0};
  /// 请求完成标志（由简化版 HandleInterrupt 在中断上下文中设置）
  volatile bool request_completed_{false};
};

}  // namespace virtio::blk
