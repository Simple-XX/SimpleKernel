use clap::ValueEnum;
use std::path::{Path, PathBuf};

#[derive(Clone, Copy, Debug, ValueEnum)]
pub enum Arch {
    Riscv64,
    Aarch64,
}

impl Arch {
    /// 短架构名，用于路径和日志输出（如 `"riscv64"`、`"aarch64"`）。
    pub fn as_str(self) -> &'static str {
        match self {
            Self::Riscv64 => "riscv64",
            Self::Aarch64 => "aarch64",
        }
    }

    /// Rust/Cargo 编译目标，传给 `cargo build --target`。
    pub fn target_triple(self) -> &'static str {
        match self {
            Self::Riscv64 => "riscv64gc-unknown-none-elf",
            Self::Aarch64 => "aarch64-unknown-none-softfloat",
        }
    }

    /// GCC 工具链前缀，传给 Make 的 `CROSS_COMPILE=`。
    /// 与 `target_triple` 格式不同，面向 GCC 而非 Rust 构建系统。
    pub fn cross_compile(self) -> &'static str {
        match self {
            Self::Riscv64 => "riscv64-linux-gnu-",
            Self::Aarch64 => "aarch64-linux-gnu-",
        }
    }

    /// QEMU 可执行文件名（如 `"qemu-system-riscv64"`）。
    pub fn qemu_binary(self) -> String {
        format!("qemu-system-{}", self.as_str())
    }

    /// U-Boot 启动脚本内容，编译时从 `src/` 下对应的 `.txt` 文件嵌入。
    pub fn boot_script_content(self) -> &'static str {
        match self {
            Self::Riscv64 => include_str!("riscv64_boot_scr.txt"),
            Self::Aarch64 => include_str!("aarch64_boot_scr.txt"),
        }
    }

    /// GCC 编译的固件输出目录：`target/firmware/{arch}/`。
    /// 与 Cargo 输出目录（`target/{target_triple}/`）分开，避免语义混淆。
    pub fn firmware_dir(self, project_root: &Path) -> PathBuf {
        project_root
            .join("target")
            .join("firmware")
            .join(self.as_str())
    }
}
