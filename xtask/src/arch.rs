use clap::ValueEnum;
use std::path::{Path, PathBuf};

#[derive(Clone, Copy, Debug, ValueEnum)]
pub enum Arch {
    Riscv64,
    Aarch64,
}

impl Arch {
    pub fn as_str(self) -> &'static str {
        match self {
            Self::Riscv64 => "riscv64",
            Self::Aarch64 => "aarch64",
        }
    }

    pub fn target_triple(self) -> &'static str {
        match self {
            Self::Riscv64 => "riscv64gc-unknown-none-elf",
            Self::Aarch64 => "aarch64-unknown-none-softfloat",
        }
    }

    pub fn qemu_binary(self) -> String {
        format!("qemu-system-{}", self.as_str())
    }

    pub fn its_template(self) -> String {
        format!("{}_qemu_virt.its.in", self.as_str())
    }

    pub fn boot_script_template(self) -> String {
        format!("{}_boot_scr.txt", self.as_str())
    }

    pub fn cross_compile(self) -> String {
        format!("{}-linux-gnu-", self.as_str())
    }

    pub fn firmware_dir(self, project_root: &Path) -> PathBuf {
        project_root
            .join("target")
            .join(self.as_str())
            .join("firmware")
    }
}
