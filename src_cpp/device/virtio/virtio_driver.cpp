/**
 * @copyright Copyright The SimpleKernel Contributors
 */

#include "virtio/virtio_driver.hpp"

#include <etl/io_port.h>

#include <utility>

#include "expected.hpp"
#include "io_buffer.hpp"
#include "kernel_log.hpp"
#include "virtio/transport/mmio.hpp"

auto VirtioDriver::MatchStatic([[maybe_unused]] DeviceNode& node) -> bool {
  return node.mmio_base != 0 && node.mmio_size != 0;
}

auto VirtioDriver::Probe(DeviceNode& node) -> Expected<void> {
  if (node.mmio_size == 0) {
    klog::Err("VirtioDriver: FDT reg property missing size for node '{}'",
              node.name);
    return std::unexpected(Error(ErrorCode::kInvalidArgument));
  }
  auto ctx = mmio_helper::Prepare(node, node.mmio_size);
  if (!ctx) {
    return std::unexpected(ctx.error());
  }

  etl::io_port_ro<uint32_t> magic_reg{reinterpret_cast<void*>(ctx->base)};
  if (magic_reg.read() != virtio::kMmioMagicValue) {
    return std::unexpected(Error(ErrorCode::kNotSupported));
  }

  // 读取 device_id
  etl::io_port_ro<uint32_t> device_id_reg{reinterpret_cast<void*>(
      ctx->base +
      std::to_underlying(virtio::MmioTransport::MmioReg::kDeviceId))};
  const auto device_id = static_cast<DeviceId>(device_id_reg.read());

  switch (device_id) {
    case DeviceId::kBlock: {
      if (blk_device_count_ >= kMaxBlkDevices) {
        klog::Warn(
            "VirtioDriver: blk device pool full, device at {:#x} skipped",
            ctx->base);
        return std::unexpected(Error(ErrorCode::kOutOfMemory));
      }
      const size_t idx = blk_device_count_;

      // 分配 DMA buffer
      dma_buffers_[idx] = kstd::make_unique<IoBuffer>(kMinDmaBufferSize);
      if (!dma_buffers_[idx] || !dma_buffers_[idx]->IsValid() ||
          dma_buffers_[idx]->GetBuffer().size() < kMinDmaBufferSize) {
        klog::Err("VirtioDriver: failed to allocate DMA buffer at {:#x}",
                  ctx->base);
        return std::unexpected(Error(ErrorCode::kOutOfMemory));
      }

      // Allocate slot DMA buffer
      auto [slot_size, slot_align] =
          virtio::blk::VirtioBlk<>::GetRequiredSlotMemSize();
      slot_buffers_[idx] = kstd::make_unique<IoBuffer>(slot_size, slot_align);
      if (!slot_buffers_[idx] || !slot_buffers_[idx]->IsValid()) {
        klog::Err("VirtioDriver: failed to allocate slot DMA buffer at {:#x}",
                  ctx->base);
        dma_buffers_[idx].reset();
        return std::unexpected(Error(ErrorCode::kOutOfMemory));
      }

      uint64_t extra_features =
          static_cast<uint64_t>(virtio::blk::BlkFeatureBit::kSegMax) |
          static_cast<uint64_t>(virtio::blk::BlkFeatureBit::kSizeMax) |
          static_cast<uint64_t>(virtio::blk::BlkFeatureBit::kBlkSize) |
          static_cast<uint64_t>(virtio::blk::BlkFeatureBit::kFlush) |
          static_cast<uint64_t>(virtio::blk::BlkFeatureBit::kGeometry);

      auto vq_dma = dma_buffers_[idx]->ToDmaRegion();
      auto slot_dma = slot_buffers_[idx]->ToDmaRegion();

      auto result = virtio::blk::VirtioBlk<>::Create(
          ctx->base, vq_dma, slot_dma, IdentityVirtToPhys, kDefaultQueueCount,
          kDefaultQueueSize, extra_features);
      if (!result.has_value()) {
        klog::Err("VirtioDriver: VirtioBlk Create failed at {:#x}", ctx->base);
        dma_buffers_[idx].reset();
        slot_buffers_[idx].reset();
        return std::unexpected(Error(result.error().code));
      }

      blk_devices_[idx].emplace(std::move(*result));
      node.type = DeviceType::kBlock;
      irqs_[idx] = node.irq;

      // Register adapter in pool and expose via DeviceNode.
      if (blk_adapter_count_ < kMaxBlkDevices) {
        const auto adapter_idx = static_cast<uint32_t>(blk_adapter_count_);
        blk_adapters_[blk_adapter_count_].emplace(&blk_devices_[idx].value(),
                                                  adapter_idx);
        node.block_device = &blk_adapters_[blk_adapter_count_].value();
        ++blk_adapter_count_;
      } else {
        klog::Warn(
            "VirtioDriver: blk adapter pool full, device at {:#x} skipped",
            ctx->base);
      }

      ++blk_device_count_;
      klog::Info(
          "VirtioDriver: block device at {:#x}, capacity={} sectors, irq={}",
          ctx->base, blk_devices_[idx].value().GetCapacity(), irqs_[idx]);
      return {};
    }

    default:
      return std::unexpected(Error(ErrorCode::kNotSupported));
  }
}
