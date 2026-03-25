//! Page table abstraction for RISC-V Sv39 and AArch64 (4KB granule).
//!
//! # Architecture layout
//!
//! | Target    | Levels | PTE layout                        |
//! |-----------|--------|-----------------------------------|
//! | riscv64   | 3      | Sv39: [63:54] res | [53:10] PPN | [9:0] flags |
//! | aarch64   | 4      | ARMv8: output addr [47:12] + attrs |
//! | host/test | 3      | mirrors Sv39 for unit-test parity  |

use bitflags::bitflags;

use crate::memory::address::PhysAddr;

// ─────────────────────────────────────────────────────────────────────────────
// PageFlags
// ─────────────────────────────────────────────────────────────────────────────

bitflags! {
    /// Architecture-independent page-table entry flags.
    ///
    /// Bit positions match RISC-V Sv39 directly.  The aarch64 `impl` block
    /// translates these to ARM descriptor bits when constructing a PTE.
    #[derive(Debug, Clone, Copy, PartialEq, Eq)]
    pub struct PageFlags: u64 {
        const VALID    = 1 << 0;
        const READ     = 1 << 1;
        const WRITE    = 1 << 2;
        const EXECUTE  = 1 << 3;
        const USER     = 1 << 4;
        const GLOBAL   = 1 << 5;
        const ACCESSED = 1 << 6;
        const DIRTY    = 1 << 7;
    }
}

impl PageFlags {
    /// 内核读写数据映射 (V | R | W | G | A | D)。
    #[inline]
    pub fn kernel_rw() -> Self {
        Self::VALID | Self::READ | Self::WRITE | Self::GLOBAL | Self::ACCESSED | Self::DIRTY
    }

    /// 内核读-执行映射 (V | R | X | G | A)。
    #[inline]
    pub fn kernel_rx() -> Self {
        Self::VALID | Self::READ | Self::EXECUTE | Self::GLOBAL | Self::ACCESSED
    }

    /// 内核只读映射 (V | R | G | A)。
    #[inline]
    pub fn kernel_ro() -> Self {
        Self::VALID | Self::READ | Self::GLOBAL | Self::ACCESSED
    }

    /// 内核读写执行映射 (V | R | W | X | G | A | D)。
    ///
    /// 用于初期 identity mapping，后续可细化为 text=RX, data=RW。
    #[inline]
    pub fn kernel_rwx() -> Self {
        Self::VALID
            | Self::READ
            | Self::WRITE
            | Self::EXECUTE
            | Self::GLOBAL
            | Self::ACCESSED
            | Self::DIRTY
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// PageTableEntry
// ─────────────────────────────────────────────────────────────────────────────

/// A single hardware page-table entry (64-bit).
#[derive(Debug, Clone, Copy)]
#[repr(transparent)]
pub struct PageTableEntry(u64);

// Sv39 PPN mask: bits [53:10]
const PPN_MASK: u64 = 0x003F_FFFF_FFFF_FC00;

// ── RISC-V Sv39 impl ─────────────────────────────────────────────────────────
#[cfg(all(not(test), target_arch = "riscv64"))]
impl PageTableEntry {
    /// Construct a leaf PTE mapping `paddr` with `flags`.
    ///
    /// PPN occupies bits [53:10] → ppn = (paddr >> 12) << 10.
    #[inline]
    pub fn new(paddr: PhysAddr, flags: PageFlags) -> Self {
        let ppn = ((paddr.as_usize() as u64) >> 12) << 10;
        Self(ppn | flags.bits())
    }

    /// Return the physical address encoded in this PTE.
    #[inline]
    pub fn paddr(self) -> PhysAddr {
        PhysAddr::new((((self.0 & PPN_MASK) >> 10) << 12) as usize)
    }

    /// Return the raw flags byte (bits [7:0]).
    #[inline]
    pub fn flags(self) -> PageFlags {
        PageFlags::from_bits_truncate(self.0 & 0xFF)
    }

    /// Is the VALID bit set?
    #[inline]
    pub fn is_valid(self) -> bool {
        self.0 & PageFlags::VALID.bits() != 0
    }

    /// Is this a leaf entry (R, W, or X set)?
    ///
    /// Non-leaf (intermediate) Sv39 entries have R=W=X=0.
    #[inline]
    pub fn is_leaf(self) -> bool {
        self.0 & (PageFlags::READ | PageFlags::WRITE | PageFlags::EXECUTE).bits() != 0
    }

    /// Zeroed (invalid) entry.
    #[inline]
    pub fn empty() -> Self {
        Self(0)
    }
}

// ── AArch64 4KB granule impl ──────────────────────────────────────────────────
#[cfg(all(not(test), target_arch = "aarch64"))]
impl PageTableEntry {
    // AArch64 descriptor constants
    const VALID_BIT: u64 = 1 << 0;
    const TABLE_BIT: u64 = 1 << 1; // 0b11 in [1:0] = table/page descriptor
    const AF_BIT: u64 = 1 << 10; // Access Flag
    const SH_INNER: u64 = 0b11 << 8; // Inner Shareable
    const MAIR_IDX0: u64 = 0b000 << 2; // MAIR index 0
    const AP_RO: u64 = 0b10 << 6; // AP[2:1] = 0b10 → EL1 RO
    const AP_RW: u64 = 0b00 << 6; // AP[2:1] = 0b00 → EL1 RW
    const PXN_BIT: u64 = 1 << 53; // Privileged Execute-Never
    const UXN_BIT: u64 = 1 << 54; // Unprivileged Execute-Never
    const OUTPUT_ADDR_MASK: u64 = 0x0000_FFFF_FFFF_F000;

    /// Construct a page (leaf) descriptor at L3.
    pub fn new(paddr: PhysAddr, flags: PageFlags) -> Self {
        let mut bits = (paddr.as_usize() as u64 & Self::OUTPUT_ADDR_MASK)
            | Self::VALID_BIT
            | Self::TABLE_BIT // page descriptor at L3 uses bit[1]=1
            | Self::AF_BIT
            | Self::SH_INNER
            | Self::MAIR_IDX0;

        // Access permissions
        if flags.contains(PageFlags::WRITE) {
            bits |= Self::AP_RW;
        } else {
            bits |= Self::AP_RO;
        }

        // Execute permissions: suppress execution unless EXECUTE flag is set
        if !flags.contains(PageFlags::EXECUTE) {
            bits |= Self::PXN_BIT | Self::UXN_BIT;
        }

        Self(bits)
    }

    /// Construct a table (non-leaf) descriptor pointing to the next-level page table.
    pub fn new_table(paddr: PhysAddr) -> Self {
        let bits =
            (paddr.as_usize() as u64 & Self::OUTPUT_ADDR_MASK) | Self::VALID_BIT | Self::TABLE_BIT;
        Self(bits)
    }

    /// Return the output (physical) address stored in the descriptor.
    #[inline]
    pub fn paddr(self) -> PhysAddr {
        PhysAddr::new((self.0 & Self::OUTPUT_ADDR_MASK) as usize)
    }

    /// Decode descriptor bits back to `PageFlags`.
    pub fn flags(self) -> PageFlags {
        let mut f = PageFlags::empty();
        if self.is_valid() {
            f |= PageFlags::VALID;
        }
        // AP[2:1]: RW = 0b00, RO = 0b10
        let ap = (self.0 >> 6) & 0b11;
        f |= PageFlags::READ;
        if ap == 0b00 {
            f |= PageFlags::WRITE;
        }
        if self.0 & Self::PXN_BIT == 0 {
            f |= PageFlags::EXECUTE;
        }
        if self.0 & Self::AF_BIT != 0 {
            f |= PageFlags::ACCESSED;
        }
        f
    }

    #[inline]
    pub fn is_valid(self) -> bool {
        self.0 & Self::VALID_BIT != 0
    }

    /// Leaf at L3: valid + table-bit set + AF set (page descriptor).
    /// At levels 0-2 entries with table-bit set are table descriptors (non-leaf).
    /// We detect leaf by checking that AF is set (only page descriptors set AF here).
    #[inline]
    pub fn is_leaf(self) -> bool {
        self.is_valid() && (self.0 & Self::AF_BIT != 0)
    }

    #[inline]
    pub fn empty() -> Self {
        Self(0)
    }
}

// ── Host / test fallback (mirrors Sv39) ──────────────────────────────────────
// Used when: running tests (any arch), OR targeting a non-riscv64/non-aarch64 host.
#[cfg(any(test, not(any(target_arch = "riscv64", target_arch = "aarch64"))))]
impl PageTableEntry {
    #[inline]
    pub fn new(paddr: PhysAddr, flags: PageFlags) -> Self {
        let ppn = ((paddr.as_usize() as u64) >> 12) << 10;
        Self(ppn | flags.bits())
    }

    #[inline]
    pub fn paddr(self) -> PhysAddr {
        PhysAddr::new((((self.0 & PPN_MASK) >> 10) << 12) as usize)
    }

    #[inline]
    pub fn flags(self) -> PageFlags {
        PageFlags::from_bits_truncate(self.0 & 0xFF)
    }

    #[inline]
    pub fn is_valid(self) -> bool {
        self.0 & PageFlags::VALID.bits() != 0
    }

    #[inline]
    pub fn is_leaf(self) -> bool {
        self.0 & (PageFlags::READ | PageFlags::WRITE | PageFlags::EXECUTE).bits() != 0
    }

    #[inline]
    pub fn empty() -> Self {
        Self(0)
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// PageTable (only available outside test; depends on FrameTracker + alloc)
// ─────────────────────────────────────────────────────────────────────────────

#[cfg(not(test))]
mod inner {
    use super::{PageFlags, PageTableEntry};
    use crate::config::PAGE_SIZE;
    use crate::error::{ErrorCode, KResult};
    use crate::memory::address::{PhysAddr, VirtAddr};
    use crate::memory::frame::FrameTracker;

    /// Number of PTEs per 4KB page (512 for 64-bit entries).
    pub const ENTRIES_PER_PAGE: usize = PAGE_SIZE / 8;

    /// Number of page-table levels.
    #[cfg(target_arch = "aarch64")]
    pub const PT_LEVELS: usize = 4;
    #[cfg(not(target_arch = "aarch64"))]
    pub const PT_LEVELS: usize = 3;

    /// Extract the 9-bit VPN index for `level` from a virtual address.
    ///
    /// Level 0 = leaf (bits [20:12]), Level PT_LEVELS-1 = root.
    #[inline]
    pub fn vpn_index(va: VirtAddr, level: usize) -> usize {
        (va.as_usize() >> (12 + level * 9)) & 0x1FF
    }

    /// Walk a physical address to the PTE array it holds.
    ///
    /// # Safety
    /// `paddr` must point to a valid, page-aligned frame that was allocated by
    /// `FrameTracker`.  The caller must ensure no aliasing `&mut` exists.
    unsafe fn pte_array(paddr: PhysAddr) -> &'static mut [PageTableEntry; ENTRIES_PER_PAGE] {
        // SAFETY: contract stated above.
        unsafe { &mut *(paddr.as_usize() as *mut [PageTableEntry; ENTRIES_PER_PAGE]) }
    }

    /// Multi-level page table.
    ///
    /// Owns the root frame and all intermediate frames allocated during walks.
    /// When dropped, all frames are returned to the allocator automatically.
    pub struct PageTable {
        root: FrameTracker,
        frames: alloc::vec::Vec<FrameTracker>,
    }

    impl PageTable {
        /// Allocate a new, empty page table (root frame zeroed by `FrameTracker`).
        pub fn new() -> KResult<Self> {
            let root = FrameTracker::alloc()?;
            Ok(Self {
                root,
                frames: alloc::vec::Vec::new(),
            })
        }

        /// Physical address of the root page-table frame.
        #[inline]
        pub fn root_paddr(&self) -> PhysAddr {
            self.root.paddr()
        }

        /// Walk from root to the leaf PTE for `va`, allocating intermediate
        /// nodes as needed.
        pub fn find_or_create_pte(&mut self, va: VirtAddr) -> KResult<&'static mut PageTableEntry> {
            let mut paddr = self.root.paddr();

            // Traverse from the root level down to level 1 (level 0 is the leaf).
            for level in (1..PT_LEVELS).rev() {
                // SAFETY: paddr is a valid frame we own (or just allocated below).
                let table = unsafe { pte_array(paddr) };
                let idx = vpn_index(va, level);
                let pte = &mut table[idx];

                if !pte.is_valid() {
                    // Allocate and wire up a new intermediate frame.
                    let frame = FrameTracker::alloc()?;
                    let frame_paddr = frame.paddr();

                    #[cfg(target_arch = "aarch64")]
                    {
                        *pte = PageTableEntry::new_table(frame_paddr);
                    }
                    #[cfg(not(target_arch = "aarch64"))]
                    {
                        // Non-leaf: only VALID, no R/W/X.
                        *pte = PageTableEntry::new(frame_paddr, PageFlags::VALID);
                    }

                    self.frames.push(frame);
                }

                // Descend into the next-level table.
                paddr = pte.paddr();
            }

            // Level 0 — the leaf PTE.
            // SAFETY: paddr is a valid frame.
            let table = unsafe { pte_array(paddr) };
            let idx = vpn_index(va, 0);
            Ok(&mut table[idx])
        }

        /// Walk from root to the leaf PTE for `va` **without** allocating.
        pub fn find_pte(&self, va: VirtAddr) -> Option<&'static PageTableEntry> {
            let mut paddr = self.root.paddr();

            for level in (1..PT_LEVELS).rev() {
                // SAFETY: paddr is a valid frame we allocated.
                let table = unsafe { pte_array(paddr) };
                let idx = vpn_index(va, level);
                let pte = &table[idx];
                if !pte.is_valid() {
                    return None;
                }
                paddr = pte.paddr();
            }

            // SAFETY: paddr is a valid frame.
            let table = unsafe { pte_array(paddr) };
            let idx = vpn_index(va, 0);
            Some(&table[idx])
        }

        /// Map virtual page `va` → physical page `pa` with `flags`.
        ///
        /// Returns `VmMapFailed` if the page is already mapped.
        pub fn map_page(&mut self, va: VirtAddr, pa: PhysAddr, flags: PageFlags) -> KResult<()> {
            let pte = self.find_or_create_pte(va)?;
            if pte.is_valid() {
                return Err(ErrorCode::VmMapFailed);
            }
            *pte = PageTableEntry::new(pa, flags);
            Ok(())
        }

        /// Unmap virtual page `va`.
        ///
        /// Returns the previously mapped physical address, or `VmPageNotMapped`
        /// if the page was not mapped.
        pub fn unmap_page(&mut self, va: VirtAddr) -> KResult<PhysAddr> {
            let pte = self.find_or_create_pte(va)?;
            if !pte.is_valid() {
                return Err(ErrorCode::VmPageNotMapped);
            }
            let old_pa = pte.paddr();
            *pte = PageTableEntry::empty();
            Ok(old_pa)
        }

        /// Query the mapping for `va`.
        ///
        /// Returns `(PhysAddr, PageFlags)` if the page is mapped and a leaf
        /// entry exists.
        pub fn get_mapping(&self, va: VirtAddr) -> Option<(PhysAddr, PageFlags)> {
            let pte = self.find_pte(va)?;
            if pte.is_valid() && pte.is_leaf() {
                Some((pte.paddr(), pte.flags()))
            } else {
                None
            }
        }
    }
}

#[cfg(not(test))]
pub use inner::PageTable;

// ─────────────────────────────────────────────────────────────────────────────
// activate_page_table
// ─────────────────────────────────────────────────────────────────────────────

/// Activate `pt` as the current address-space root.
///
/// # Safety
/// The caller must ensure that `pt` provides a complete, correct mapping for
/// all code/data that will execute after this call returns.

// RISC-V Sv39
#[cfg(all(not(test), target_arch = "riscv64"))]
pub unsafe fn activate_page_table(pt: &PageTable) {
    let ppn = pt.root_paddr().as_usize() >> 12;
    let satp = (8usize << 60) | ppn; // MODE = 8 → Sv39
    // SAFETY: caller guarantees pt is valid.
    unsafe {
        core::arch::asm!(
            "csrw satp, {satp}",
            "sfence.vma",
            satp = in(reg) satp,
        );
    }
}

// AArch64
#[cfg(all(not(test), target_arch = "aarch64"))]
pub unsafe fn activate_page_table(pt: &PageTable) {
    let ttbr = pt.root_paddr().as_usize() as u64;
    // SAFETY: caller guarantees pt is valid.
    unsafe {
        core::arch::asm!(
            "msr ttbr0_el1, {ttbr}",
            "isb",
            "tlbi vmalle1",
            "dsb sy",
            "isb",
            ttbr = in(reg) ttbr,
        );
        // Enable MMU via SCTLR_EL1.M (bit 0)
        let mut sctlr: u64;
        core::arch::asm!("mrs {sctlr}, sctlr_el1", sctlr = out(reg) sctlr);
        sctlr |= 1;
        core::arch::asm!(
            "msr sctlr_el1, {sctlr}",
            "isb",
            sctlr = in(reg) sctlr,
        );
    }
}

// Host / unknown arch — no-op (satisfies the test build)
#[cfg(all(not(test), not(target_arch = "riscv64"), not(target_arch = "aarch64")))]
pub unsafe fn activate_page_table(_pt: &PageTable) {
    // no-op on host builds
}

// ─────────────────────────────────────────────────────────────────────────────
// Unit tests (run on host; only exercise PageFlags + PageTableEntry encoding)
// ─────────────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn pte_roundtrip() {
        // Use a well-known page-aligned physical address.
        let pa = PhysAddr::new(0x8020_0000);
        let flags = PageFlags::VALID | PageFlags::READ | PageFlags::WRITE;

        let pte = PageTableEntry::new(pa, flags);

        // Physical address must survive a round-trip.
        assert_eq!(pte.paddr(), pa, "paddr round-trip failed");

        // Flags must be preserved (bits [7:0]).
        let recovered = pte.flags();
        assert!(
            recovered.contains(PageFlags::VALID),
            "VALID not recovered: {:?}",
            recovered
        );
        assert!(
            recovered.contains(PageFlags::READ),
            "READ not recovered: {:?}",
            recovered
        );
        assert!(
            recovered.contains(PageFlags::WRITE),
            "WRITE not recovered: {:?}",
            recovered
        );
    }

    #[test]
    fn pte_empty_is_invalid() {
        let pte = PageTableEntry::empty();
        assert!(!pte.is_valid(), "empty PTE must not be valid");
        assert!(!pte.is_leaf(), "empty PTE must not be a leaf");
    }

    #[test]
    fn page_flags_presets() {
        let rw = PageFlags::kernel_rw();
        assert!(rw.contains(PageFlags::VALID));
        assert!(rw.contains(PageFlags::READ));
        assert!(rw.contains(PageFlags::WRITE));
        assert!(rw.contains(PageFlags::GLOBAL));
        assert!(rw.contains(PageFlags::ACCESSED));
        assert!(rw.contains(PageFlags::DIRTY));
        assert!(!rw.contains(PageFlags::EXECUTE));

        let rx = PageFlags::kernel_rx();
        assert!(rx.contains(PageFlags::VALID));
        assert!(rx.contains(PageFlags::READ));
        assert!(rx.contains(PageFlags::EXECUTE));
        assert!(rx.contains(PageFlags::GLOBAL));
        assert!(rx.contains(PageFlags::ACCESSED));
        assert!(!rx.contains(PageFlags::WRITE));

        let ro = PageFlags::kernel_ro();
        assert!(ro.contains(PageFlags::VALID));
        assert!(ro.contains(PageFlags::READ));
        assert!(ro.contains(PageFlags::GLOBAL));
        assert!(ro.contains(PageFlags::ACCESSED));
        assert!(!ro.contains(PageFlags::WRITE));
        assert!(!ro.contains(PageFlags::EXECUTE));

        let rwx = PageFlags::kernel_rwx();
        assert!(rwx.contains(PageFlags::VALID));
        assert!(rwx.contains(PageFlags::READ));
        assert!(rwx.contains(PageFlags::WRITE));
        assert!(rwx.contains(PageFlags::EXECUTE));
        assert!(rwx.contains(PageFlags::GLOBAL));
        assert!(rwx.contains(PageFlags::ACCESSED));
        assert!(rwx.contains(PageFlags::DIRTY));
    }

    #[test]
    fn pte_is_valid_and_leaf() {
        let pa = PhysAddr::new(0x0000_1000);
        // An entry with READ set is a leaf.
        let leaf = PageTableEntry::new(pa, PageFlags::VALID | PageFlags::READ);
        assert!(leaf.is_valid());
        assert!(leaf.is_leaf());

        // An entry with only VALID (Sv39 intermediate node) is not a leaf.
        let intermediate = PageTableEntry::new(pa, PageFlags::VALID);
        assert!(intermediate.is_valid());
        assert!(!intermediate.is_leaf());
    }
}
