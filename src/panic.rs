#![allow(dead_code)]

use crate::elf::KernelElf;
use crate::memory::address::VirtAddr;
use crate::sync::SpinLock;
use core::fmt::Write;
use spin::Once;

static KERNEL_ELF: Once<KernelElf> = Once::new();

const MAX_OBSERVERS: usize = 4;
const MAX_BACKTRACE_DEPTH: usize = 16;

pub struct PanicEvent<'a> {
    pub reason: &'a str,
    pub file: &'a str,
    pub line: u32,
    pub pc: VirtAddr,
}

/// 希望在内核 panic 时收到通知的组件所实现的 trait。
pub trait PanicObserver: Send + Sync {
    fn on_panic(&self, event: &PanicEvent<'_>);
}

struct ObserverRegistry {
    slots: [Option<&'static dyn PanicObserver>; MAX_OBSERVERS],
}

impl ObserverRegistry {
    const fn new() -> Self {
        Self {
            slots: [None; MAX_OBSERVERS],
        }
    }
}

static OBSERVERS: SpinLock<ObserverRegistry> =
    SpinLock::new(ObserverRegistry::new(), "panic_observers");

/// 初始化用于回溯解析的 ELF 符号表。
///
/// # Safety
/// `elf_addr` 必须指向一个有效的 ELF64 二进制，并且在整个内核生命周期内保持映射。
pub unsafe fn init_elf(elf_addr: u64) {
    // SAFETY: 调用者保证该 ELF 地址有效
    match unsafe { KernelElf::new(elf_addr) } {
        Ok(elf) => {
            KERNEL_ELF.call_once(|| elf);
        }
        Err(_) => {
            #[cfg(not(test))]
            crate::logging::raw_put("WARNING: failed to parse kernel ELF for backtrace\n");
        }
    }
}

pub fn register_observer(observer: &'static dyn PanicObserver) {
    let mut registry = OBSERVERS.lock();
    for slot in registry.slots.iter_mut() {
        if slot.is_none() {
            *slot = Some(observer);
            return;
        }
    }
}

fn notify_observers(event: &PanicEvent<'_>) {
    if let Some(registry) = OBSERVERS.try_lock() {
        for slot in &registry.slots {
            if let Some(obs) = slot {
                obs.on_panic(event);
            }
        }
    }
}

use crate::fmt_buf::FmtBuf;

/// 核心 panic 处理器。打印位置、消息、回溯，并通知观察者。
#[cfg(not(test))]
pub fn handle_panic(info: &core::panic::PanicInfo<'_>) -> ! {
    use crate::logging::raw_put;

    raw_put("\x1b[31mPANIC\x1b[0m at ");

    let (file, line) = if let Some(loc) = info.location() {
        let mut buf = FmtBuf::new();
        let _ = write!(buf, "{}:{}", loc.file(), loc.line());
        raw_put(buf.as_str());
        (loc.file(), loc.line())
    } else {
        raw_put("<unknown>");
        ("<unknown>", 0)
    };

    raw_put(": ");
    let mut msg_buf = FmtBuf::new();
    let _ = write!(msg_buf, "{}", info.message());
    raw_put(msg_buf.as_str());
    raw_put("\n");

    dump_backtrace();

    let event = PanicEvent {
        reason: msg_buf.as_str(),
        file,
        line,
        pc: VirtAddr::new(0),
    };
    notify_observers(&event);

    loop {
        core::hint::spin_loop();
    }
}

/// 使用 DWARF `.eh_frame` 数据（通过 `unwinding` crate）遍历栈，
/// 并打印每个栈帧的返回地址及可选的符号名。
#[cfg(all(not(test), target_os = "none"))]
fn dump_backtrace() {
    use crate::logging::raw_put;
    use core::sync::atomic::{AtomicUsize, Ordering};
    use unwinding::abi::{_Unwind_Backtrace, _Unwind_GetIP, UnwindContext, UnwindReasonCode};

    raw_put("  backtrace:\n");

    static DEPTH: AtomicUsize = AtomicUsize::new(0);
    DEPTH.store(0, Ordering::Relaxed);

    extern "C" fn trace_callback(
        ctx: &UnwindContext<'_>,
        _arg: *mut core::ffi::c_void,
    ) -> UnwindReasonCode {
        let ip = _Unwind_GetIP(ctx);
        if ip == 0 {
            return UnwindReasonCode::NORMAL_STOP;
        }

        let depth = DEPTH.fetch_add(1, Ordering::Relaxed);
        if depth >= MAX_BACKTRACE_DEPTH {
            return UnwindReasonCode::NORMAL_STOP;
        }

        let mut buf = FmtBuf::new();
        let _ = write!(buf, "    #{}: 0x{:016X}", depth, ip);
        crate::logging::raw_put(buf.as_str());

        if let Some(elf) = KERNEL_ELF.get() {
            if let Some(name) = elf.lookup_symbol(ip as u64) {
                crate::logging::raw_put(" - ");
                let mut sym_buf = FmtBuf::new();
                let _ = write!(sym_buf, "{:#}", rustc_demangle::demangle(name));
                crate::logging::raw_put(sym_buf.as_str());
            }
        }
        crate::logging::raw_put("\n");

        UnwindReasonCode::NO_REASON
    }

    _Unwind_Backtrace(trace_callback, core::ptr::null_mut());
}

#[cfg(all(not(test), not(target_os = "none")))]
fn dump_backtrace() {}

#[cfg(all(not(test), target_os = "none"))]
pub fn raw_dump_stack() {
    dump_backtrace();
}

#[cfg(all(not(test), not(target_os = "none")))]
pub fn raw_dump_stack() {}

#[cfg(test)]
pub fn handle_panic(_info: &core::panic::PanicInfo<'_>) -> ! {
    loop {
        core::hint::spin_loop();
    }
}

#[cfg(test)]
pub fn raw_dump_stack() {}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn panic_event_fields() {
        let event = PanicEvent {
            reason: "test panic",
            file: "src/main.rs",
            line: 42,
            pc: VirtAddr::new(0x8020_0000),
        };
        assert_eq!(event.reason, "test panic");
        assert_eq!(event.line, 42);
    }

    #[test]
    fn fmt_buf_basic() {
        let mut buf = FmtBuf::new();
        let _ = write!(buf, "hello {}", 42);
        assert_eq!(buf.as_str(), "hello 42");
    }

    #[test]
    fn fmt_buf_overflow() {
        let mut buf = FmtBuf::new();
        for _ in 0..100 {
            let _ = write!(buf, "overflow!");
        }
        assert!(buf.pos <= 256);
        assert!(!buf.as_str().is_empty());
    }
}
