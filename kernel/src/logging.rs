use core::fmt;
use core::fmt::Write;
use core::sync::atomic::{AtomicBool, AtomicU64, Ordering};

use spin::Mutex;

const ANSI_RESET: &str = "\x1b[0m";
const ANSI_RED: &str = "\x1b[31m";
const ANSI_GREEN: &str = "\x1b[32m";
const ANSI_YELLOW: &str = "\x1b[33m";
const ANSI_CYAN: &str = "\x1b[36m";

const FMT_BUF_SIZE: usize = 256;

static CONSOLE_LOCK: Mutex<()> = Mutex::new(());
static LOG_SEQ: AtomicU64 = AtomicU64::new(0);
static LOGGER_INIT: AtomicBool = AtomicBool::new(false);
static LOGGER: KernelLogger = KernelLogger;

fn put_str(s: &str) {
    #[cfg(target_arch = "riscv64")]
    crate::arch::riscv64::console::puts(s);
    #[cfg(target_arch = "aarch64")]
    crate::arch::aarch64::console::puts(s);
}

fn level_color(level: log::Level) -> &'static str {
    match level {
        log::Level::Debug | log::Level::Trace => ANSI_GREEN,
        log::Level::Info => ANSI_CYAN,
        log::Level::Warn => ANSI_YELLOW,
        log::Level::Error => ANSI_RED,
    }
}

fn level_label(level: log::Level) -> &'static str {
    match level {
        log::Level::Debug | log::Level::Trace => "DEBUG",
        log::Level::Info => "INFO ",
        log::Level::Warn => "WARN ",
        log::Level::Error => "ERROR",
    }
}

struct FmtBuf {
    buf: [u8; FMT_BUF_SIZE],
    pos: usize,
}

impl FmtBuf {
    const fn new() -> Self {
        Self {
            buf: [0; FMT_BUF_SIZE],
            pos: 0,
        }
    }

    fn as_str(&self) -> &str {
        core::str::from_utf8(&self.buf[..self.pos]).unwrap_or("")
    }
}

impl fmt::Write for FmtBuf {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        let bytes = s.as_bytes();
        let available = FMT_BUF_SIZE.saturating_sub(self.pos);
        let to_copy = available.min(bytes.len());
        if to_copy > 0 {
            self.buf[self.pos..self.pos + to_copy].copy_from_slice(&bytes[..to_copy]);
            self.pos += to_copy;
        }
        Ok(())
    }
}

struct KernelLogger;

impl log::Log for KernelLogger {
    fn enabled(&self, _metadata: &log::Metadata<'_>) -> bool {
        true
    }

    fn log(&self, record: &log::Record<'_>) {
        if !self.enabled(record.metadata()) {
            return;
        }

        let seq = LOG_SEQ.fetch_add(1, Ordering::Relaxed);
        let core_id = crate::per_cpu::current_core_id();
        let level = record.level();

        let mut buf = FmtBuf::new();
        let _ = write!(&mut buf, "{}", record.args());

        let _guard = CONSOLE_LOCK.lock();
        put_str(level_color(level));
        let mut hdr = FmtBuf::new();
        let _ = write!(&mut hdr, "[{}][{} {}] ", seq, core_id, level_label(level));
        put_str(hdr.as_str());
        put_str(buf.as_str());
        put_str(ANSI_RESET);
        put_str("\n");
    }

    fn flush(&self) {}
}

pub fn init() {
    if LOGGER_INIT.swap(true, Ordering::AcqRel) {
        return;
    }
    if log::set_logger(&LOGGER).is_ok() {
        log::set_max_level(log::LevelFilter::Debug);
    }
}

pub fn flush() {}

pub fn raw_put(msg: &str) {
    put_str(msg);
}
