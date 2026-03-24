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
}

#[derive(Subcommand)]
enum Commands {
    Build(ArchArgs),
    Run(ArchArgs),
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
            build::build_kernel(&sh, &project_root, args.arch)?;
        }
        Commands::Firmware(args) => {
            firmware::build_firmware(&sh, &project_root, args.arch)?;
        }
        Commands::Run(args) => {
            let arch = args.arch;
            let kernel_elf_path = build::build_kernel(&sh, &project_root, arch)?;
            let boot_dir = build::prepare_boot_directory(&project_root, arch)?;
            let rootfs_path = build::ensure_rootfs_image(&sh, &boot_dir)?;
            let dtb_path = qemu::dump_qemu_dtb(&sh, arch, &boot_dir, &rootfs_path)?;
            qemu::generate_fit_image(
                arch,
                &sh,
                &project_root,
                &boot_dir,
                &kernel_elf_path,
                &dtb_path,
            )?;
            qemu::generate_boot_script(arch, &sh, &project_root, &boot_dir)?;
            qemu::setup_tftp(&boot_dir);
            firmware::ensure_firmware_exists(&project_root, arch)?;
            qemu::launch_qemu(
                &sh,
                arch,
                &project_root,
                &boot_dir,
                &kernel_elf_path,
                &rootfs_path,
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
