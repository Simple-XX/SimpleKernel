const UARTDR: usize = 0x00;
const UARTFR: usize = 0x18;
const UARTFR_TXFF: u32 = 1 << 5;
const PL011_BASE: usize = 0x0900_0000;

pub fn putchar(c: u8) {
    // SAFETY: PL011_BASE + offsets are valid MMIO registers on QEMU virt aarch64
    unsafe {
        let fr = (PL011_BASE + UARTFR) as *const u32;
        while core::ptr::read_volatile(fr) & UARTFR_TXFF != 0 {
            core::hint::spin_loop();
        }
        let dr = (PL011_BASE + UARTDR) as *mut u32;
        core::ptr::write_volatile(dr, c as u32);
    }
}

pub fn puts(s: &str) {
    for byte in s.bytes() {
        putchar(byte);
    }
}
