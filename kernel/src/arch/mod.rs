#[cfg(target_arch = "aarch64")]
mod aarch64;
#[cfg(target_arch = "riscv64")]
mod riscv64;

#[cfg(target_arch = "aarch64")]
pub fn bootstrap() -> ! {
    aarch64::bootstrap()
}

#[cfg(target_arch = "riscv64")]
pub fn bootstrap() -> ! {
    riscv64::bootstrap()
}
