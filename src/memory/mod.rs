pub mod address;
pub mod page_table;

#[cfg(not(test))]
pub mod frame;
#[cfg(not(test))]
pub mod heap;

#[cfg(not(test))]
use address::{PhysAddr, VirtAddr};
#[cfg(not(test))]
use page_table::{PageFlags, PageTable};

/// Identity mapping: phys_to_virt is identity for now.
#[cfg(not(test))]
#[allow(dead_code)]
pub fn phys_to_virt(pa: PhysAddr) -> VirtAddr {
    VirtAddr::new(pa.as_usize())
}

/// Identity mapping: virt_to_phys is identity for now.
#[cfg(not(test))]
#[allow(dead_code)]
pub fn virt_to_phys(va: VirtAddr) -> PhysAddr {
    PhysAddr::new(va.as_usize())
}

/// Map a range of pages with identity mapping (VA == PA).
#[cfg(not(test))]
fn identity_map_range(
    pt: &mut PageTable,
    start: PhysAddr,
    end: PhysAddr,
    flags: PageFlags,
) -> crate::error::KResult<()> {
    let mut addr = start.align_down();
    let end_aligned = end.align_up();
    while addr.as_usize() < end_aligned.as_usize() {
        pt.map_page(VirtAddr::new(addr.as_usize()), addr, flags)?;
        addr = addr + crate::config::PAGE_SIZE;
    }
    Ok(())
}

/// Primary memory initialization — called by BSP (bootstrap processor).
///
/// 1. Initialize heap allocator (static BSS region)
/// 2. Initialize frame allocator (physical memory from FDT)
/// 3. Create kernel page table with identity mapping
/// 4. Enable paging
#[cfg(not(test))]
pub fn memory_init() {
    // Step 1: Heap — must come first so we can use Vec/Box
    unsafe { heap::heap_init() };

    // Step 2: Frame allocator
    let info = crate::per_cpu::BASIC_INFO
        .get()
        .expect("BASIC_INFO not initialized");
    let mem_start = info.physical_memory_addr;
    let mem_size = info.physical_memory_size;
    let kernel_end = info.kernel_addr + info.kernel_size;

    // Allocatable region starts after kernel image (page-aligned)
    let alloc_start = kernel_end.align_up();
    let alloc_size = mem_size - (alloc_start - mem_start);

    unsafe { frame::frame_init(alloc_start, alloc_size) };

    // Step 3: Create kernel page table with identity mapping
    let mut pt = PageTable::new().expect("failed to create kernel page table");

    // Identity-map the entire physical memory region
    identity_map_range(
        &mut pt,
        mem_start,
        mem_start + mem_size,
        PageFlags::kernel_rw(),
    )
    .expect("failed to identity-map memory");

    log::info!(
        "MemoryInit: kernel mapped {}-{}",
        mem_start,
        mem_start + mem_size
    );

    // Step 4: Paging activation deferred to P4 (requires trap handler to debug page faults).
    // The page table structure is fully built and ready.
    // TODO(P4): after setting up stvec trap handler, call:
    //   unsafe { page_table::activate_page_table(&pt) };
    log::info!("MemoryInit: page table ready (activation deferred to P4)");

    // Leak the kernel page table — it must live forever
    core::mem::forget(pt);
}

/// Secondary core memory init — loads kernel page table into satp/TTBR.
/// TODO(P4): called by bootstrap_smp()
#[cfg(not(test))]
#[allow(dead_code)]
pub fn memory_init_smp() {
    // Secondary cores share the BSP's page table.
}

/// Map MMIO region, returns virtual address.
/// TODO(P6): DeviceInit calls this to map device registers.
#[cfg(not(test))]
#[allow(dead_code)]
pub fn map_mmio(paddr: PhysAddr, _size: usize) -> crate::error::KResult<VirtAddr> {
    // Identity mapping: VA == PA
    Ok(VirtAddr::new(paddr.as_usize()))
}
