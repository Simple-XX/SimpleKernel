use core::fmt::Write;
use core::sync::atomic::{AtomicBool, AtomicU64, Ordering};

use spin::Mutex;

const ANSI_RESET: &str = "\x1b[0m";
const ANSI_RED: &str = "\x1b[31m";
const ANSI_GREEN: &str = "\x1b[32m";
const ANSI_YELLOW: &str = "\x1b[33m";
const ANSI_CYAN: &str = "\x1b[36m";
const ANSI_GRAY: &str = "\x1b[90m";

static CONSOLE_LOCK: Mutex<()> = Mutex::new(());
static LOG_SEQ: AtomicU64 = AtomicU64::new(0);
static LOGGER_INIT: AtomicBool = AtomicBool::new(false);
static LOGGER: KernelLogger = KernelLogger;

fn put_str(s: &str) {
    #[cfg(all(target_arch = "riscv64", not(test)))]
    crate::arch::riscv64::console::puts(s);
    #[cfg(all(target_arch = "aarch64", not(test)))]
    crate::arch::aarch64::console::puts(s);
    #[cfg(any(test, not(any(target_arch = "riscv64", target_arch = "aarch64"))))]
    {
        let _ = s;
    }
}

fn level_color(level: log::Level) -> &'static str {
    match level {
        log::Level::Trace => ANSI_GRAY,
        log::Level::Debug => ANSI_GREEN,
        log::Level::Info => ANSI_CYAN,
        log::Level::Warn => ANSI_YELLOW,
        log::Level::Error => ANSI_RED,
    }
}

fn level_label(level: log::Level) -> &'static str {
    match level {
        log::Level::Trace => "TRACE",
        log::Level::Debug => "DEBUG",
        log::Level::Info => "INFO ",
        log::Level::Warn => "WARN ",
        log::Level::Error => "ERROR",
    }
}

use crate::fmt_buf::FmtBuf;

struct KernelLogger;

impl log::Log for KernelLogger {
    fn enabled(&self, metadata: &log::Metadata<'_>) -> bool {
        metadata.level() <= log::max_level()
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

        let mut hdr = FmtBuf::new();
        let _ = write!(
            &mut hdr,
            "{}[{}][{} {}] ",
            level_color(level),
            seq,
            core_id,
            level_label(level)
        );

        let _guard = CONSOLE_LOCK.lock();
        put_str(hdr.as_str());
        put_str(buf.as_str());
        if buf.is_truncated() {
            put_str("...[truncated]");
        }
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
