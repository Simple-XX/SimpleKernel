pub mod console;
pub mod init;

pub fn bootstrap(argc: i32, argv: *const *const u8) -> ! {
    init::arch_init(argc, argv);
    crate::phase2_smoke_test();
    loop {
        core::hint::spin_loop();
    }
}
