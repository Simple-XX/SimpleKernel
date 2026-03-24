pub mod console;
pub mod init;

pub fn bootstrap(argc: i32, argv: *const *const u8) -> ! {
    init::arch_init(argc, argv);
    loop {
        core::hint::spin_loop();
    }
}
