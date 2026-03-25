use std::fs;
use std::path::{Path, PathBuf};
use xshell::{Shell, cmd};

use crate::Result;
use crate::arch::Arch;

pub fn build_kernel(sh: &Shell, project_root: &Path, arch: Arch) -> Result<PathBuf> {
    println!("[xtask] Building kernel for {}...", arch.as_str());
    let target = arch.target_triple();
    cmd!(
        sh,
        "cargo build -Z build-std=core,compiler_builtins,alloc -Z build-std-features=compiler-builtins-mem --target {target}"
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

pub fn generate_debug_files(sh: &Shell, arch: Arch, kernel_elf: &Path) -> Result<()> {
    println!("[xtask] Generating debug files...");
    let cross = arch.cross_compile();
    let objdump = format!("{cross}objdump");
    let readelf = format!("{cross}readelf");
    let nm = format!("{cross}nm");
    let objcopy = format!("{cross}objcopy");

    let objdump_out = kernel_elf.with_extension("objdump");
    let readelf_out = kernel_elf.with_extension("readelf");
    let nm_out = kernel_elf.with_extension("nm");
    let bin_out = kernel_elf.with_extension("bin");

    match cmd!(sh, "{objdump} -D {kernel_elf}").output() {
        Ok(out) => fs::write(&objdump_out, &out.stdout)?,
        Err(e) => eprintln!("[xtask] warning: objdump failed: {e}"),
    }
    match cmd!(sh, "{readelf} -a {kernel_elf}").output() {
        Ok(out) => fs::write(&readelf_out, &out.stdout)?,
        Err(e) => eprintln!("[xtask] warning: readelf failed: {e}"),
    }
    match cmd!(sh, "{nm} -a {kernel_elf}").output() {
        Ok(out) => fs::write(&nm_out, &out.stdout)?,
        Err(e) => eprintln!("[xtask] warning: nm failed: {e}"),
    }
    match cmd!(sh, "{objcopy} -O binary {kernel_elf} {bin_out}").output() {
        Ok(_) => {}
        Err(e) => eprintln!("[xtask] warning: objcopy failed: {e}"),
    }

    Ok(())
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
