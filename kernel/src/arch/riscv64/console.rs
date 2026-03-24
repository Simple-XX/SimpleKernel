pub fn putchar(c: u8) {
    sbi_rt::console_write_byte(c);
}

pub fn puts(s: &str) {
    for byte in s.bytes() {
        putchar(byte);
    }
}
