#![no_std]
#![no_main]
#![feature(alloc_error_handler)]
#![feature(custom_test_frameworks)]

mod arch;
mod lang_items;

#[used]
static RODATA_SENTINEL: [u8; 1] = [0x42];

#[used]
static mut DATA_SENTINEL: u64 = 1;

#[used]
static mut BSS_SENTINEL: u64 = 0;

#[unsafe(no_mangle)]
pub extern "C" fn _start(argc: i32, argv: *const *const u8) -> ! {
    let _ = argc;
    let _ = argv;

    unsafe {
        core::ptr::write_volatile(core::ptr::addr_of_mut!(DATA_SENTINEL), 2);
        let _ = core::ptr::read_volatile(core::ptr::addr_of!(RODATA_SENTINEL));
        let _ = core::ptr::read_volatile(core::ptr::addr_of!(BSS_SENTINEL));
    }

    arch::bootstrap();
}
