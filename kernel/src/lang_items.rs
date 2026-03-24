use core::alloc::Layout;
use core::panic::PanicInfo;

#[panic_handler]
fn panic(_info: &PanicInfo<'_>) -> ! {
    loop {
        core::hint::spin_loop();
    }
}

#[alloc_error_handler]
fn alloc_error(_layout: Layout) -> ! {
    loop {
        core::hint::spin_loop();
    }
}
