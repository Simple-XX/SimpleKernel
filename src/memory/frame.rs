use crate::config::PAGE_SIZE;
use crate::error::{ErrorCode, KResult};
use crate::memory::address::PhysAddr;
use crate::sync::SpinLock;

/// Global frame allocator — wraps buddy_system_allocator::FrameAllocator.
/// Operates in units of page frames (PAGE_SIZE bytes each).
static FRAME_ALLOCATOR: SpinLock<FrameAllocatorInner> =
    SpinLock::new(FrameAllocatorInner::new(), "frame_alloc");

struct FrameAllocatorInner {
    allocator: buddy_system_allocator::FrameAllocator<32>,
    initialized: bool,
}

impl FrameAllocatorInner {
    const fn new() -> Self {
        Self {
            allocator: buddy_system_allocator::FrameAllocator::new(),
            initialized: false,
        }
    }
}

/// Initialize the frame allocator with available physical memory.
///
/// `start` must be page-aligned. The region `[start, start+size)` becomes
/// available for frame allocation.
///
/// # Safety
/// The memory region must be valid, not overlap with kernel/heap, and
/// this must be called exactly once.
pub unsafe fn frame_init(start: PhysAddr, size: usize) {
    let mut alloc = FRAME_ALLOCATOR.lock();
    assert!(!alloc.initialized, "frame_init called twice");
    assert!(start.is_aligned(), "frame_init: start not page-aligned");

    let start_frame = start.as_usize() / PAGE_SIZE;
    let end_frame = start_frame + size / PAGE_SIZE;
    alloc.allocator.add_frame(start_frame, end_frame);
    alloc.initialized = true;

    log::info!(
        "FrameInit: {} MB available from {}",
        size / (1024 * 1024),
        start
    );
}

/// Physical frame RAII guard — automatically returns frame to allocator on Drop.
/// Eliminates "forgot to free_frame" physical memory leaks (rCore pattern).
pub struct FrameTracker {
    paddr: PhysAddr,
}

impl FrameTracker {
    /// Allocate a single physical frame (PAGE_SIZE bytes), zeroed.
    pub fn alloc() -> KResult<Self> {
        let mut alloc = FRAME_ALLOCATOR.lock();
        if !alloc.initialized {
            return Err(ErrorCode::VmAllocationFailed);
        }
        let frame_num = alloc.allocator.alloc(1).ok_or(ErrorCode::OutOfMemory)?;
        let paddr = PhysAddr::new(frame_num * PAGE_SIZE);

        // Zero the frame
        unsafe {
            core::ptr::write_bytes(paddr.as_usize() as *mut u8, 0, PAGE_SIZE);
        }

        Ok(Self { paddr })
    }

    pub fn paddr(&self) -> PhysAddr {
        self.paddr
    }
}

impl Drop for FrameTracker {
    fn drop(&mut self) {
        let mut alloc = FRAME_ALLOCATOR.lock();
        let frame_num = self.paddr.as_usize() / PAGE_SIZE;
        alloc.allocator.dealloc(frame_num, 1);
    }
}
