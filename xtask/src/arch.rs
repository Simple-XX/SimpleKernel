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

    pub fn cross_compile(self) -> &'static str {
        match self {
            Self::Riscv64 => "riscv64-linux-gnu-",
            Self::Aarch64 => "aarch64-linux-gnu-",
        }
    }

    pub fn qemu_binary(self) -> String {
        format!("qemu-system-{}", self.as_str())
    }

    pub fn boot_script_content(self) -> &'static str {
        match self {
            Self::Riscv64 => include_str!("riscv64_boot_scr.txt"),
            Self::Aarch64 => include_str!("aarch64_boot_scr.txt"),
        }
    }

    pub fn firmware_dir(self, project_root: &Path) -> PathBuf {
        project_root
            .join("target")
            .join("firmware")
            .join(self.as_str())
    }
}
