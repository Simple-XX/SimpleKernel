pub fn bootstrap() -> ! {
    loop {
        core::hint::spin_loop();
    }
}
