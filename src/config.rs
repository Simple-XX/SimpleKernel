pub const MAX_CORE_COUNT: usize = 4;

/// 页大小，单位字节
pub const PAGE_SIZE: usize = 4096;

/// 页大小的位数（log2(PAGE_SIZE)）
pub const PAGE_SIZE_BITS: usize = 12;

/// Kernel heap size: 4 MB (backed by static BSS array)
pub const KERNEL_HEAP_SIZE: usize = 4 * 1024 * 1024;
