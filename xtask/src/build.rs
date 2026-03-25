use std::fs;
use std::path::{Path, PathBuf};
use std::str;
use xshell::{Shell, cmd};

use crate::Result;
use crate::arch::Arch;

/// 编译内核 ELF，返回产物路径。
///
/// `release` 为 `true` 时附加 `--release`，输出路径切换到 `release/` 子目录。
pub fn build_kernel(sh: &Shell, project_root: &Path, arch: Arch, release: bool) -> Result<PathBuf> {
    println!("[xtask] Building kernel for {}...", arch.as_str());
    let target = arch.target_triple();
    let mut build_cmd = cmd!(
        sh,
        "cargo build -Z build-std=core,compiler_builtins,alloc -Z build-std-features=compiler-builtins-mem --target {target}"
    );
    if release {
        build_cmd = build_cmd.arg("--release");
    }
    build_cmd.run()?;

    let profile_dir = if release { "release" } else { "debug" };
    let kernel_elf_path = project_root
        .join("target")
        .join(arch.target_triple())
        .join(profile_dir)
        .join("simplekernel");
    if !kernel_elf_path.exists() {
        return Err(format!("kernel ELF not found at {}", kernel_elf_path.display()).into());
    }

    Ok(kernel_elf_path)
}

/// 从 `rustc` sysroot 解析 `llvm-tools` 中指定工具的绝对路径。
///
/// 工具由 `rust-toolchain.toml` 的 `components` 保证已安装，路径形如
/// `{sysroot}/lib/rustlib/{host}/bin/{tool}`。
fn llvm_tool_path(sh: &Shell, tool: &str) -> Result<PathBuf> {
    let sysroot = cmd!(sh, "rustc --print sysroot").output()?;
    let sysroot = PathBuf::from(str::from_utf8(&sysroot.stdout)?.trim());

    let version_info = cmd!(sh, "rustc -vV").output()?;
    let host = str::from_utf8(&version_info.stdout)?
        .lines()
        .find(|l| l.starts_with("host:"))
        .and_then(|l| l.split_whitespace().nth(1))
        .ok_or("rustc -vV did not report a host triple")?;

    Ok(sysroot
        .join("lib")
        .join("rustlib")
        .join(host)
        .join("bin")
        .join(tool))
}

/// 生成内核调试文件，与内核 ELF 放在同一目录：
/// - `.objdump` — 完整反汇编（`llvm-objdump -D`）
/// - `.readelf` — ELF 头及节信息（`llvm-readobj -a`）
/// - `.nm`      — 符号表（`llvm-nm -a`）
/// - `.bin`     — 纯二进制（`llvm-objcopy -O binary`）
///
/// 使用 `llvm-tools` 中的工具，原生支持所有目标架构，
/// 无需额外安装 GCC cross 工具链。各工具失败时只打警告，不中断构建。
pub fn generate_debug_files(sh: &Shell, kernel_elf: &Path) -> Result<()> {
    println!("[xtask] Generating debug files...");

    let objdump = llvm_tool_path(sh, "llvm-objdump")?;
    let readobj = llvm_tool_path(sh, "llvm-readobj")?;
    let nm = llvm_tool_path(sh, "llvm-nm")?;
    let objcopy = llvm_tool_path(sh, "llvm-objcopy")?;

    let objdump_out = kernel_elf.with_extension("objdump");
    let readelf_out = kernel_elf.with_extension("readelf");
    let nm_out = kernel_elf.with_extension("nm");
    let bin_out = kernel_elf.with_extension("bin");

    match cmd!(sh, "{objdump} -D {kernel_elf}").output() {
        Ok(out) => fs::write(&objdump_out, &out.stdout)?,
        Err(e) => eprintln!("[xtask] warning: llvm-objdump failed: {e}"),
    }
    match cmd!(sh, "{readobj} -a {kernel_elf}").output() {
        Ok(out) => fs::write(&readelf_out, &out.stdout)?,
        Err(e) => eprintln!("[xtask] warning: llvm-readobj failed: {e}"),
    }
    match cmd!(sh, "{nm} -a {kernel_elf}").output() {
        Ok(out) => fs::write(&nm_out, &out.stdout)?,
        Err(e) => eprintln!("[xtask] warning: llvm-nm failed: {e}"),
    }
    match cmd!(sh, "{objcopy} -O binary {kernel_elf} {bin_out}").output() {
        Ok(_) => {}
        Err(e) => eprintln!("[xtask] warning: llvm-objcopy failed: {e}"),
    }

    Ok(())
}

/// 创建启动产物目录 `target/{triple}/{profile}/boot/`，按需 `mkdir -p`。
pub fn prepare_boot_directory(project_root: &Path, arch: Arch, release: bool) -> Result<PathBuf> {
    let profile_dir = if release { "release" } else { "debug" };
    let boot_dir = project_root
        .join("target")
        .join(arch.target_triple())
        .join(profile_dir)
        .join("boot");
    println!(
        "[xtask] Preparing output directory {}...",
        boot_dir.display()
    );
    fs::create_dir_all(&boot_dir)?;
    Ok(boot_dir)
}

/// 确保 rootfs FAT32 镜像存在。
///
/// 首次运行时用 `dd` + `mkfs.fat` 在 `boot_dir` 下创建 64 MiB 空镜像；
/// 已存在则直接返回，跳过创建。
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
