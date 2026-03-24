use std::fs;
use std::path::{Path, PathBuf};
use xshell::{Shell, cmd};

use crate::Result;
use crate::arch::Arch;

pub fn build_kernel(sh: &Shell, project_root: &Path, arch: Arch) -> Result<PathBuf> {
    println!("[xtask] Building kernel for {}...", arch.as_str());
    let target_json = arch.target_json();
    cmd!(
        sh,
        "cargo build -Z build-std=core,compiler_builtins,alloc -Z build-std-features=compiler-builtins-mem --target {target_json}"
    )
    .run()?;

    let kernel_elf_path = project_root
        .join("target")
        .join(arch.target_triple())
        .join("debug")
        .join("simplekernel");
    if !kernel_elf_path.exists() {
        return Err(format!("kernel ELF not found at {}", kernel_elf_path.display()).into());
    }

    Ok(kernel_elf_path)
}

pub fn prepare_boot_directory(project_root: &Path, arch: Arch) -> Result<PathBuf> {
    let boot_dir = project_root
        .join("target")
        .join(arch.target_triple())
        .join("debug")
        .join("boot");
    println!(
        "[xtask] Preparing output directory {}...",
        boot_dir.display()
    );
    fs::create_dir_all(&boot_dir)?;
    Ok(boot_dir)
}

pub fn ensure_rootfs_image(sh: &Shell, boot_dir: &Path) -> Result<PathBuf> {
    let rootfs_path = boot_dir.join("rootfs.img");
    if rootfs_path.exists() {
        println!("[xtask] rootfs.img already exists, skipping creation.");
        return Ok(rootfs_path);
    }

    println!("[xtask] Creating rootfs image...");
    let dd_out = format!("of={}", rootfs_path.display());
    cmd!(sh, "dd if=/dev/zero {dd_out} bs=1M count=64").run()?;
    cmd!(sh, "mkfs.fat -F 32 {rootfs_path}").run()?;
    Ok(rootfs_path)
}
