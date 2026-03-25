use core::alloc::Layout;
use core::panic::PanicInfo;

#[panic_handler]
fn panic(info: &PanicInfo<'_>) -> ! {
    crate::panic::handle_panic(info);
}

#[alloc_error_handler]
fn alloc_error(_layout: Layout) -> ! {
    crate::logging::raw_put("KERNEL PANIC: alloc error\n");
    loop {
        core::hint::spin_loop();
    }
}
