use core::alloc::Layout;
use core::fmt::Write;
use core::panic::PanicInfo;

struct PanicBuf {
    buf: [u8; 128],
    pos: usize,
}

impl PanicBuf {
    const fn new() -> Self {
        Self {
            buf: [0; 128],
            pos: 0,
        }
    }
    fn as_str(&self) -> &str {
        core::str::from_utf8(&self.buf[..self.pos]).unwrap_or("")
    }
}

impl Write for PanicBuf {
    fn write_str(&mut self, s: &str) -> core::fmt::Result {
        let bytes = s.as_bytes();
        let avail = 128usize.saturating_sub(self.pos);
        let n = avail.min(bytes.len());
        if n > 0 {
            self.buf[self.pos..self.pos + n].copy_from_slice(&bytes[..n]);
            self.pos += n;
        }
        Ok(())
    }
}

#[panic_handler]
fn panic(info: &PanicInfo<'_>) -> ! {
    crate::logging::raw_put("KERNEL PANIC: ");
    if let Some(loc) = info.location() {
        let mut buf = PanicBuf::new();
        let _ = write!(buf, "{}:{}", loc.file(), loc.line());
        crate::logging::raw_put(buf.as_str());
    }
    crate::logging::raw_put(" - ");
    let mut buf = PanicBuf::new();
    let _ = write!(buf, "{}", info.message());
    crate::logging::raw_put(buf.as_str());
    crate::logging::raw_put("\n");
    loop {
        core::hint::spin_loop();
    }
}

#[alloc_error_handler]
fn alloc_error(_layout: Layout) -> ! {
    crate::logging::raw_put("KERNEL PANIC: alloc error\n");
    loop {
        core::hint::spin_loop();
    }
}
