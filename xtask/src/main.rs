//! xtask — 内核构建工具
//!
//! 替代 CMake 的宿主机构建脚本，通过 `cargo xtask <subcommand>` 调用。
//!
//! 子命令：
//! - `build`    — 编译内核并生成调试文件
//! - `run`      — 编译并在 QEMU 中运行
//! - `debug`    — 编译并在 QEMU 中以调试模式运行（暂停 CPU，等待 GDB 连接）
//! - `firmware` — 编译第三方固件（OpenSBI / U-Boot / OP-TEE / ATF）

mod arch;
mod build;
mod firmware;
mod qemu;

use clap::{Args, Parser, Subcommand};
use std::path::PathBuf;
use std::process;

pub use arch::Arch;

type DynError = Box<dyn std::error::Error>;
type Result<T> = std::result::Result<T, DynError>;

#[derive(Parser)]
#[command(name = "xtask")]
struct Cli {
    #[command(subcommand)]
    command: Commands,
}

#[derive(Args, Clone)]
struct ArchArgs {
    #[arg(long, value_enum, default_value = "riscv64")]
    arch: Arch,
    #[arg(long)]
    release: bool,
}

#[derive(Subcommand)]
enum Commands {
    Build(ArchArgs),
    Run(ArchArgs),
    /// 启动 QEMU 并暂停 CPU，等待 GDB 在 localhost:1234 连接
    Debug(ArchArgs),
    Firmware(ArchArgs),
}

fn main() {
    if let Err(error) = run() {
        eprintln!("[xtask] error: {error}");
        process::exit(1);
    }
}

fn run() -> Result<()> {
    let cli = Cli::parse();
    let project_root = project_root();
    let sh = xshell::Shell::new()?;
    sh.change_dir(&project_root);

    match cli.command {
        Commands::Build(args) => {
            let kernel_elf_path = build::build_kernel(&sh, &project_root, args.arch, args.release)?;
            build::generate_debug_files(&sh, &kernel_elf_path)?;
        }
        Commands::Firmware(args) => {
            firmware::build_firmware(&sh, &project_root, args.arch)?;
        }
        Commands::Run(args) => {
            let arch = args.arch;
            // 提前检查固件，避免内核编译完成后才发现固件缺失。
            firmware::ensure_firmware_exists(&project_root, arch)?;
            let kernel_elf_path = build::build_kernel(&sh, &project_root, arch, args.release)?;
            build::generate_debug_files(&sh, &kernel_elf_path)?;
            let boot_dir = build::prepare_boot_directory(&project_root, arch, args.release)?;
            let rootfs_path = build::ensure_rootfs_image(&sh, &boot_dir)?;
            let dtb_path = qemu::dump_qemu_dtb(&sh, arch, &boot_dir, &rootfs_path)?;
            qemu::generate_fit_image(arch, &sh, &boot_dir, &kernel_elf_path, &dtb_path)?;
            qemu::generate_boot_script(arch, &sh, &boot_dir)?;
            qemu::setup_tftp(&boot_dir);
            qemu::launch_qemu(
                &sh,
                arch,
                &project_root,
                &boot_dir,
                &kernel_elf_path,
                &rootfs_path,
                false,
            )?;
        }
        Commands::Debug(args) => {
            let arch = args.arch;
            firmware::ensure_firmware_exists(&project_root, arch)?;
            let kernel_elf_path = build::build_kernel(&sh, &project_root, arch, args.release)?;
            build::generate_debug_files(&sh, &kernel_elf_path)?;
            let boot_dir = build::prepare_boot_directory(&project_root, arch, args.release)?;
            let rootfs_path = build::ensure_rootfs_image(&sh, &boot_dir)?;
            let dtb_path = qemu::dump_qemu_dtb(&sh, arch, &boot_dir, &rootfs_path)?;
            qemu::generate_fit_image(arch, &sh, &boot_dir, &kernel_elf_path, &dtb_path)?;
            qemu::generate_boot_script(arch, &sh, &boot_dir)?;
            qemu::setup_tftp(&boot_dir);
            qemu::launch_qemu(
                &sh,
                arch,
                &project_root,
                &boot_dir,
                &kernel_elf_path,
                &rootfs_path,
                true,
            )?;
        }
    }

    Ok(())
}

fn project_root() -> PathBuf {
    std::path::Path::new(env!("CARGO_MANIFEST_DIR"))
        .parent()
        .expect("xtask must be a direct workspace member")
        .to_path_buf()
}
