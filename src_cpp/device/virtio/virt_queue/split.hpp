/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#pragma once

#include <utility>

#include "expected.hpp"
#include "io_buffer.hpp"
#include "virtio/virt_queue/misc.hpp"
#include "virtio/virt_queue/virtqueue_base.hpp"

namespace virtio {

/**
 * @brief Split Virtqueue 管理类
 *
 * 管理 split virtqueue 的描述符分配/释放、缓冲区提交、已用缓冲区回收。
 * 使用预分配的 DMA 内存，自身不进行任何堆内存分配。
 *
 * 内存布局（在 DMA 缓冲区中连续排列）：
 * ```
 * [Descriptor Table]  aligned to 16
 * [Available Ring]    aligned to 2
 * [Used Ring]         aligned to 4
 * ```
 *
 * @warning 非线程安全：此类的所有方法均不是线程安全的。
 *          如果多个线程/核需要访问同一个
 * virtqueue，调用者必须使用外部同步机制（如自旋锁或互斥锁）。
 * @warning 单生产者-单消费者：描述符分配和提交应由同一线程执行，
 *                          已用缓冲区回收应由另一线程执行（通常在中断处理程序中）。
 *
 * @see cpu_io::Wmb/Rmb for barrier semantics
 * @see virtio-v1.2#2.7
 */
class SplitVirtqueue final : public VirtqueueBase {
 public:
  /**
   * @brief Descriptor Flags
   * @see virtio-v1.2#2.7.5 The Virtqueue Descriptor Table
   */
  enum class DescFlags : uint16_t {
    /// 标记缓冲区通过 next 字段继续
    kDescFNext = 1,
    /// 标记缓冲区为设备只写(否则为设备只读)
    kDescFWrite = 2,
    /// 标记缓冲区包含描述符列表(间接描述符)
    kDescFIndirect = 4
  };

  /**
   * @brief Available Ring Flags
   * @see virtio-v1.2#2.7.6 The Virtqueue Available Ring
   */
  enum class AvailFlags : uint16_t {
    /// 设备应该不发送中断
    kAvailFNoInterrupt = 1
  };

  /**
   * @brief Used Ring Flags
   * @see virtio-v1.2#2.7.8 The Virtqueue Used Ring
   */
  enum class UsedFlags : uint16_t {
    /// 驱动不需要通知
    kUsedFNoNotify = 1
  };

  /**
   * @brief Virtqueue 描述符表条目
   *
   * 描述符表引用驱动程序使用的缓冲区。每个描述符描述一个缓冲区，
   * 该缓冲区对设备是只读的("设备可读")或只写的("设备可写")。
   *
   * @note 16 字节对齐
   * @see virtio-v1.2#2.7.5 The Virtqueue Descriptor Table
   */
  struct [[gnu::packed]] Desc {
    /// Descriptor Table 对齐要求(字节)
    static constexpr size_t kAlign = 16;
    /// 缓冲区的客户机物理地址 (little-endian)
    uint64_t addr;
    /// 缓冲区长度(字节) (little-endian)
    uint32_t len;
    /// 标志位: DescFlags (little-endian)
    uint16_t flags;
    /// 下一个描述符的索引(当 flags & kDescFNext 时有效) (little-endian)
    uint16_t next;
  };

  /**
   * @brief Virtqueue Available Ring
   *
   * Available Ring 用于驱动程序向设备提供缓冲区。
   * 驱动程序将描述符链的头部放入环中。
   *
   * @note 实际 ring[] 大小由 queue_size 决定
   * @note 2 字节对齐
   * @see virtio-v1.2#2.7.6 The Virtqueue Available Ring
   */
  struct [[gnu::packed]] Avail {
    /// Available Ring 对齐要求(字节)
    static constexpr size_t kAlign = 2;
    /// 标志位: AvailFlags (little-endian)
    uint16_t flags;
    /// 驱动程序将下一个描述符条目放入环中的位置(模 queue_size) (little-endian)
    uint16_t idx;
    /// 可用描述符头索引数组 ring[queue_size] (little-endian)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    uint16_t ring[];
#pragma GCC diagnostic pop

    /**
     * @brief 获取 used_event 字段的指针
     * @param queue_size 队列大小
     * @return used_event 字段指针
     * @note 仅当协商 VIRTIO_F_EVENT_IDX 特性时使用
     * @see virtio-v1.2#2.7.10 Available Buffer Notification Suppression
     */
    [[nodiscard]] auto used_event(uint16_t queue_size) volatile
        -> volatile uint16_t* {
      return ring + queue_size;
    }

    [[nodiscard]] auto used_event(uint16_t queue_size) const volatile -> const
        volatile uint16_t* {
      return ring + queue_size;
    }
  };

  /**
   * @brief Virtqueue Used Ring 元素
   *
   * Used Ring 中的每个条目是一个 (id, len) 对。
   *
   * @see virtio-v1.2#2.7.8 The Virtqueue Used Ring
   */
  struct [[gnu::packed]] UsedElem {
    /// 描述符链头的索引 (little-endian)
    uint32_t id;
    /// 设备写入描述符链的总字节数 (little-endian)
    uint32_t len;
  };

  /**
   * @brief Virtqueue Used Ring
   *
   * Used Ring 是设备完成缓冲区处理后返回它们的地方。
   * 它只由设备写入，由驱动程序读取。
   *
   * @note 实际 ring[] 大小由 queue_size 决定
   * @note 4 字节对齐
   * @see virtio-v1.2#2.7.8 The Virtqueue Used Ring
   */
  struct [[gnu::packed]] Used {
    /// Used Ring 对齐要求(字节)
    static constexpr size_t kAlign = 4;
    /// 标志位: UsedFlags (little-endian)
    uint16_t flags;
    /// 设备将下一个描述符条目放入环中的位置(模 queue_size) (little-endian)
    uint16_t idx;
    /// 已用描述符元素数组 ring[queue_size]
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    UsedElem ring[];
#pragma GCC diagnostic pop

    /**
     * @brief 获取 avail_event 字段的指针
     * @param queue_size 队列大小
     * @return avail_event 字段指针
     * @note 仅当协商 VIRTIO_F_EVENT_IDX 特性时使用
     * @see virtio-v1.2#2.7.10 Available Buffer Notification Suppression
     */
    [[nodiscard]] auto avail_event(uint16_t queue_size) volatile
        -> volatile uint16_t* {
      // avail_event 位于 ring[queue_size] 之后，需要先转换为字节指针计算偏移
      auto* byte_ptr = reinterpret_cast<volatile uint8_t*>(ring);
      return reinterpret_cast<volatile uint16_t*>(byte_ptr + sizeof(UsedElem) *
                                                                 queue_size);
    }

    [[nodiscard]] auto avail_event(uint16_t queue_size) const volatile -> const
        volatile uint16_t* {
      const auto* byte_ptr = reinterpret_cast<const volatile uint8_t*>(ring);
      return reinterpret_cast<const volatile uint16_t*>(
          byte_ptr + sizeof(UsedElem) * queue_size);
    }
  };

  /**
   * @brief 计算给定队列大小所需的 DMA 内存字节数
   *
   * @param queue_size 队列大小（必须为 2 的幂）
   * @param event_idx 是否启用 VIRTIO_F_EVENT_IDX 特性
   * @param used_align Used Ring 的对齐要求
   * @return 所需的 DMA 内存字节数
   * @see virtio-v1.2#2.6 Split Virtqueues
   */
  [[nodiscard]] static constexpr auto CalcSize(uint16_t queue_size,
                                               bool event_idx = true,
                                               size_t used_align = Used::kAlign)
      -> size_t {
    // Descriptor Table: sizeof(Desc) * queue_size
    size_t desc_total = static_cast<size_t>(sizeof(Desc)) * queue_size;

    // Available Ring: flags(2) + idx(2) + ring[N](2*N) + used_event(2, 可选)
    size_t avail_total = sizeof(uint16_t) * (2 + queue_size);
    if (event_idx) {
      avail_total += sizeof(uint16_t);
    }

    // Used Ring: flags(2) + idx(2) + ring[N](sizeof(UsedElem)*N) +
    // avail_event(2, 可选)
    size_t used_total = sizeof(uint16_t) * 2 + sizeof(UsedElem) * queue_size;
    if (event_idx) {
      used_total += sizeof(uint16_t);
    }

    // 按对齐要求排列
    size_t avail_off = AlignUp(desc_total, Avail::kAlign);
    size_t used_off = AlignUp(avail_off + avail_total, used_align);

    return used_off + used_total;
  }

  /**
   * @brief 从预分配的 DMA 缓冲区构造 SplitVirtqueue
   *
   * @param dma  DMA 区域描述符（包含虚拟地址、物理地址和大小，
   *             必须已清零，大小 >= CalcSize()）
   * @param queue_size 队列大小（必须为 2 的幂）
   * @param event_idx 是否启用 VIRTIO_F_EVENT_IDX 特性
   * @param used_align Used Ring 的对齐要求（modern = 4，legacy MMIO = 4096）
   * @see virtio-v1.2#2.7
   */
  SplitVirtqueue(const DmaRegion& dma, uint16_t queue_size, bool event_idx,
                 size_t used_align = Used::kAlign)
      : queue_size_(queue_size),
        phys_base_(dma.phys),
        event_idx_enabled_(event_idx) {
    if (!dma.IsValid() || !IsPowerOfTwo(queue_size)) {
      return;
    }

    size_t desc_total = static_cast<size_t>(sizeof(Desc)) * queue_size;
    size_t avail_total = sizeof(uint16_t) * (2 + queue_size);
    if (event_idx) {
      avail_total += sizeof(uint16_t);
    }

    desc_offset_ = 0;
    avail_offset_ = AlignUp(desc_total, Avail::kAlign);
    size_t used_total = sizeof(uint16_t) * 2 + sizeof(UsedElem) * queue_size;
    if (event_idx) {
      used_total += sizeof(uint16_t);
    }
    used_offset_ = AlignUp(avail_offset_ + avail_total, used_align);

    auto* base = dma.Data();
    desc_ = reinterpret_cast<volatile Desc*>(base + desc_offset_);
    avail_ = reinterpret_cast<volatile Avail*>(base + avail_offset_);
    used_ = reinterpret_cast<volatile Used*>(base + used_offset_);

    for (uint16_t i = 0; i < queue_size; ++i) {
      desc_[i].next = static_cast<uint16_t>(i + 1);
    }
    // 末尾描述符使用 sentinel 值，避免越界索引
    desc_[queue_size - 1].next = 0xFFFF;
    free_head_ = 0;
    num_free_ = queue_size;
    last_used_idx_ = 0;

    is_valid_ = true;
  }

  /**
   * @brief 检查 virtqueue 是否成功初始化
   */
  [[nodiscard]] auto IsValid() const -> bool { return is_valid_; }

  /**
   * @brief 从空闲链表分配一个描述符
   *
   * 从空闲描述符链表中取出一个描述符，供上层使用。
   * 调用者必须填充描述符的 addr、len、flags 和 next 字段。
   *
   * @return 成功返回描述符索引（range: 0 ~ queue_size-1）；
   *         空闲链表为空时返回 ErrorCode::kNoFreeDescriptors
   *
   * @warning 非线程安全：多个线程同时调用可能导致竞态条件
   * @see virtio-v1.2#2.7.5 The Virtqueue Descriptor Table
   * @see virtio-v1.2#2.7.13 Supplying Buffers to The Device
   */
  [[nodiscard]] auto AllocDesc() -> Expected<uint16_t> {
    if (num_free_ == 0) {
      return std::unexpected(Error{ErrorCode::kNoFreeDescriptors});
    }

    uint16_t idx = free_head_;
    free_head_ = desc_[free_head_].next;
    --num_free_;

    return idx;
  }

  /**
   * @brief 归还描述符到空闲链表
   *
   * 将不再使用的描述符放回空闲链表，供后续分配使用。
   * 对于描述符链，调用者必须按正确的顺序释放链中的每个描述符。
   *
   * @param idx 要释放的描述符索引（必须为之前分配的有效索引）
   *
   * @warning 非线程安全：多个线程同时调用可能导致竞态条件
   * @warning 释放已释放的描述符或无效索引会导致空闲链表损坏
   * @see virtio-v1.2#2.7.14 Receiving Used Buffers From The Device
   */
  auto FreeDesc(uint16_t idx) -> Expected<void> {
    if (idx >= queue_size_) {
      return std::unexpected(Error{ErrorCode::kInvalidDescriptor});
    }
    desc_[idx].next = free_head_;
    free_head_ = idx;
    ++num_free_;
    return {};
  }

  /**
   * @brief 获取描述符的可变引用
   *
   * 用于设置描述符的 addr、len、flags 和 next 字段。
   * 调用者必须确保索引有效（通过 AllocDesc() 分配）。
   *
   * @param idx 描述符索引（必须 < queue_size）
   * @return 描述符的 volatile 引用（用于与设备共享内存）
   * @see virtio-v1.2#2.7.5 The Virtqueue Descriptor Table
   */
  [[nodiscard]] auto GetDesc(uint16_t idx) -> Expected<volatile Desc*> {
    if (idx >= queue_size_) {
      return std::unexpected(Error{ErrorCode::kInvalidDescriptor});
    }
    return &desc_[idx];
  }

  /**
   * @brief 获取描述符的只读引用
   *
   * @param idx 描述符索引（必须 < queue_size）
   * @return 描述符的 const volatile 指针，失败返回错误
   */
  [[nodiscard]] auto GetDesc(uint16_t idx) const
      -> Expected<const volatile Desc*> {
    if (idx >= queue_size_) {
      return std::unexpected(Error{ErrorCode::kInvalidDescriptor});
    }
    return &desc_[idx];
  }

  /**
   * @brief 将描述符链提交到 Available Ring
   *
   * 将描述符链的头部索引放入 Available Ring，使其对设备可见。
   * 调用此方法后，调用者应使用内存屏障确保 idx 更新对设备可见，
   * 然后通过 Transport::NotifyQueue() 通知设备。
   *
   * @param head 描述符链头部索引（必须为有效的已分配描述符）
   *
   * @note 调用者必须在调用此方法前确保描述符写入已完成
   * @note 调用者必须在调用此方法后通知设备（如 Transport::NotifyQueue()）
   *
   * @see cpu_io::Wmb() 用于确保 ring 写入在 idx 更新之前对设备可见
   *
   * @warning 非线程安全：多个线程同时调用可能导致竞态条件
   * @see virtio-v1.2#2.7.13 Supplying Buffers to The Device
   */
  auto Submit(uint16_t head) -> void {
    uint16_t idx = avail_->idx;
    avail_->ring[idx % queue_size_] = head;

    // 写屏障：确保 ring 写入在 idx 更新之前对设备可见
    cpu_io::Wmb();

    avail_->idx = idx + 1;
  }

  /**
   * @brief 检查 Used Ring 中是否有已完成的缓冲区
   *
   * 通过比较驱动程序上次处理的 idx 与设备当前的 idx，
   * 判断是否有新的已处理缓冲区可供回收。
   *
   * @return true 表示有已完成的缓冲区可用，false 表示没有
   * @see virtio-v1.2#2.7.14 Receiving Used Buffers From The Device
   */
  [[nodiscard]] auto HasUsed() const -> bool {
    return last_used_idx_ != used_->idx;
  }

  /**
   * @brief 从 Used Ring 弹出一个已完成的元素
   *
   * 从 Used Ring 中取出下一个设备已处理完成的缓冲区。
   * 返回的 UsedElem 包含：
   * - id: 描述符链头索引（对应之前提交的 head）
   * - len: 设备写入的字节数（仅对 Device-writable 缓冲区有意义）
   *
   * @return 成功返回 UsedElem{id, len}；
   *         无可用元素时返回 ErrorCode::kNoUsedBuffers
   *
   * @warning 非线程安全：多个线程同时调用可能导致竞态条件
   * @see virtio-v1.2#2.7.14 Receiving Used Buffers From The Device
   */
  [[nodiscard]] auto PopUsed() -> Expected<UsedElem> {
    if (!HasUsed()) {
      return std::unexpected(Error{ErrorCode::kNoUsedBuffers});
    }

    uint16_t idx = last_used_idx_ % queue_size_;
    UsedElem elem;
    elem.id = used_->ring[idx].id;
    elem.len = used_->ring[idx].len;

    ++last_used_idx_;

    return elem;
  }

  /**
   * @brief 提交 Scatter-Gather 描述符链
   *
   * 从空闲链表分配描述符，按顺序组装 readable（设备只读）和
   * writable（设备可写） 缓冲区为描述符链，自动设置 NEXT 标志串联，并提交到
   * Available Ring。
   *
   * 描述符链顺序：[readable_0, ..., readable_N, writable_0, ..., writable_M]
   * - readable 部分：flags = kDescFNext（无 kDescFWrite）
   * - writable 部分：flags = kDescFNext | kDescFWrite
   * - 最后一个描述符清除 kDescFNext
   *
   * 调用者在调用此方法后仍需调用内存屏障 + Transport::NotifyQueue() 通知设备。
   *
   * @param readable 设备只读缓冲区数组（如请求头、写入数据）
   * @param readable_count readable 数组中的元素数量
   * @param writable 设备可写缓冲区数组（如读取数据、状态字节）
   * @param writable_count writable 数组中的元素数量
   * @return 成功返回描述符链头索引（可用作 token）；失败返回错误
   *
   * @pre readable_count + writable_count > 0
   * @pre readable_count + writable_count <= NumFree()
   * @post 描述符链已提交到 Available Ring
   *
   * @warning 非线程安全
   * @see virtio-v1.2#2.7.13 Supplying Buffers to The Device
   * @see 架构文档 §3 Scatter-Gather
   */
  [[nodiscard]] auto SubmitChain(const IoVec* readable, size_t readable_count,
                                 const IoVec* writable, size_t writable_count)
      -> Expected<uint16_t> {
    size_t total = readable_count + writable_count;
    if (total == 0) {
      return std::unexpected(Error{ErrorCode::kInvalidArgument});
    }
    if (num_free_ < static_cast<uint16_t>(total)) {
      return std::unexpected(Error{ErrorCode::kNoFreeDescriptors});
    }

    uint16_t head = free_head_;
    uint16_t prev_idx = 0xFFFF;

    for (size_t i = 0; i < readable_count; ++i) {
      uint16_t idx = free_head_;
      free_head_ = desc_[free_head_].next;
      --num_free_;

      desc_[idx].addr = readable[i].phys_addr;
      desc_[idx].len = static_cast<uint32_t>(readable[i].len);
      desc_[idx].flags = std::to_underlying(DescFlags::kDescFNext);

      if (prev_idx != 0xFFFF) {
        desc_[prev_idx].next = idx;
      }
      prev_idx = idx;
    }

    for (size_t i = 0; i < writable_count; ++i) {
      uint16_t idx = free_head_;
      free_head_ = desc_[free_head_].next;
      --num_free_;

      desc_[idx].addr = writable[i].phys_addr;
      desc_[idx].len = static_cast<uint32_t>(writable[i].len);
      desc_[idx].flags = std::to_underlying(DescFlags::kDescFNext) |
                         std::to_underlying(DescFlags::kDescFWrite);
      if (prev_idx != 0xFFFF) {
        desc_[prev_idx].next = idx;
      }
      prev_idx = idx;
    }

    desc_[prev_idx].flags =
        desc_[prev_idx].flags & ~std::to_underlying(DescFlags::kDescFNext);

    // 写屏障：确保描述符写入在 Available Ring 更新之前对设备可见
    cpu_io::Wmb();

    Submit(head);

    return head;
  }

  /**
   * @brief 释放整条描述符链
   *
   * 从链头开始，沿 next 指针遍历并释放所有描述符，直到遇到
   * 不含 kDescFNext 标志的描述符为止。
   *
   * 典型用法：在 PopUsed() 获取已完成请求的 head 后，
   * 用此方法一次性归还整条链的描述符。
   *
   * @param head 描述符链头索引
   * @return 成功或失败（如 head 索引无效）
   *
   * @warning 非线程安全
   * @see virtio-v1.2#2.7.14 Receiving Used Buffers From The Device
   */
  auto FreeChain(uint16_t head) -> Expected<void> {
    if (head >= queue_size_) {
      return std::unexpected(Error{ErrorCode::kInvalidDescriptor});
    }

    uint16_t idx = head;
    while (true) {
      if (idx >= queue_size_) {
        return std::unexpected(Error{ErrorCode::kInvalidDescriptor});
      }

      uint16_t next = desc_[idx].next;
      bool has_next =
          (desc_[idx].flags & std::to_underlying(DescFlags::kDescFNext)) != 0;

      desc_[idx].next = free_head_;
      free_head_ = idx;
      ++num_free_;

      if (!has_next) {
        break;
      }
      idx = next;
    }

    return {};
  }

  /**
   * @brief 获取描述符表的物理地址
   * @see virtio-v1.2#2.7.5
   */
  [[nodiscard]] auto DescPhys() const -> uint64_t {
    return phys_base_ + desc_offset_;
  }

  /**
   * @brief 获取 Available Ring 的物理地址
   * @see virtio-v1.2#2.7.6
   */
  [[nodiscard]] auto AvailPhys() const -> uint64_t {
    return phys_base_ + avail_offset_;
  }

  /**
   * @brief 获取 Used Ring 的物理地址
   * @see virtio-v1.2#2.7.8
   */
  [[nodiscard]] auto UsedPhys() const -> uint64_t {
    return phys_base_ + used_offset_;
  }

  /**
   * @brief 获取队列大小
   */
  [[nodiscard]] auto Size() const -> uint16_t { return queue_size_; }

  /**
   * @brief 获取当前空闲描述符数量
   */
  [[nodiscard]] auto NumFree() const -> uint16_t { return num_free_; }

  /**
   * @brief 获取 Available Ring 的 used_event 字段
   *
   * @return used_event 字段指针，未启用 EVENT_IDX 则返回 nullptr
   * @see virtio-v1.2#2.7.10
   */
  [[nodiscard]] auto AvailUsedEvent() -> volatile uint16_t* {
    return event_idx_enabled_ ? avail_->used_event(queue_size_) : nullptr;
  }

  [[nodiscard]] auto AvailUsedEvent() const -> const volatile uint16_t* {
    return event_idx_enabled_ ? avail_->used_event(queue_size_) : nullptr;
  }

  /**
   * @brief 获取 Used Ring 的 avail_event 字段
   *
   * @return avail_event 字段指针，未启用 EVENT_IDX 则返回 nullptr
   * @see virtio-v1.2#2.7.10
   */
  [[nodiscard]] auto UsedAvailEvent() -> volatile uint16_t* {
    return event_idx_enabled_ ? used_->avail_event(queue_size_) : nullptr;
  }

  [[nodiscard]] auto UsedAvailEvent() const -> const volatile uint16_t* {
    return event_idx_enabled_ ? used_->avail_event(queue_size_) : nullptr;
  }

  /**
   * @brief 检查是否启用了 VIRTIO_F_EVENT_IDX 特性
   */
  [[nodiscard]] auto EventIdxEnabled() const -> bool {
    return event_idx_enabled_;
  }

  /**
   * @brief 获取当前 Available Ring 索引
   */
  [[nodiscard]] auto AvailIdx() const -> uint16_t { return avail_->idx; }

  /**
   * @brief 获取驱动程序上次处理到的 Used Ring 索引
   */
  [[nodiscard]] auto LastUsedIdx() const -> uint16_t { return last_used_idx_; }

  /// @name 构造/析构函数
  /// @{
  SplitVirtqueue() = delete;
  SplitVirtqueue(const SplitVirtqueue&) = delete;
  auto operator=(const SplitVirtqueue&) -> SplitVirtqueue& = delete;
  auto operator=(SplitVirtqueue&&) -> SplitVirtqueue& = delete;
  SplitVirtqueue(SplitVirtqueue&& other) noexcept
      : VirtqueueBase(std::move(other)),
        desc_(other.desc_),
        avail_(other.avail_),
        used_(other.used_),
        queue_size_(other.queue_size_),
        free_head_(other.free_head_),
        num_free_(other.num_free_),
        last_used_idx_(other.last_used_idx_),
        phys_base_(other.phys_base_),
        desc_offset_(other.desc_offset_),
        avail_offset_(other.avail_offset_),
        used_offset_(other.used_offset_),
        event_idx_enabled_(other.event_idx_enabled_),
        is_valid_(other.is_valid_) {
    other.is_valid_ = false;
    other.desc_ = nullptr;
    other.avail_ = nullptr;
    other.used_ = nullptr;
  }
  ~SplitVirtqueue() = default;
  /// @}

 private:
  /// 描述符表指针（指向 DMA 内存）
  volatile Desc* desc_{nullptr};
  /// Available Ring 指针（指向 DMA 内存）
  volatile Avail* avail_{nullptr};
  /// Used Ring 指针（指向 DMA 内存）
  volatile Used* used_{nullptr};

  /// 队列大小（描述符数量，必须为 2 的幂）
  uint16_t queue_size_{0};
  /// 空闲描述符链表头索引
  uint16_t free_head_{0};
  /// 空闲描述符数量
  uint16_t num_free_{0};
  /// 上次处理到的 Used Ring 索引（用于 PopUsed）
  uint16_t last_used_idx_{0};

  /// DMA 内存物理基地址（客户机物理地址）
  uint64_t phys_base_{0};
  /// 描述符表在 DMA 内存中的偏移量（字节）
  size_t desc_offset_{0};
  /// Available Ring 在 DMA 内存中的偏移量（字节）
  size_t avail_offset_{0};
  /// Used Ring 在 DMA 内存中的偏移量（字节）
  size_t used_offset_{0};
  /// 是否启用 VIRTIO_F_EVENT_IDX 特性
  bool event_idx_enabled_{false};
  /// 初始化是否成功
  bool is_valid_{false};
};

}  // namespace virtio
