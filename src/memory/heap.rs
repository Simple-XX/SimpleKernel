use crate::config::KERNEL_HEAP_SIZE;
use buddy_system_allocator::LockedHeap;

#[global_allocator]
static HEAP_ALLOCATOR: LockedHeap<32> = LockedHeap::empty();

/// BSS-resident heap backing store.
static mut HEAP_SPACE: [u8; KERNEL_HEAP_SIZE] = [0; KERNEL_HEAP_SIZE];

/// Initialize the kernel heap allocator.
///
/// # Safety
/// Must be called exactly once, before any heap allocation.
pub unsafe fn heap_init() {
    let heap_start = unsafe { HEAP_SPACE.as_ptr() as usize };
    unsafe {
        HEAP_ALLOCATOR.lock().init(heap_start, KERNEL_HEAP_SIZE);
    }
    log::info!(
        "HeapInit: {}MB heap at {:#x}",
        KERNEL_HEAP_SIZE / (1024 * 1024),
        heap_start
    );
}
