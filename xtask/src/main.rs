mod arch;
mod build;
mod firmware;
mod qemu;

use clap::{Parser, Subcommand};
use std::fs;
use std::path::{Path, PathBuf};
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

#[derive(Subcommand)]
enum Commands {
    Build {
        #[arg(long, value_enum, default_value = "riscv64")]
        arch: Arch,
    },
    Run {
        #[arg(long, value_enum, default_value = "riscv64")]
        arch: Arch,
    },
    Firmware {
        #[arg(long, value_enum, default_value = "riscv64")]
        arch: Arch,
    },
}

fn main() {
    if let Err(error) = run() {
        eprintln!("[xtask] error: {error}");
        process::exit(1);
    }
}

fn run() -> Result<()> {
    let cli = Cli::parse();
    let project_root = find_workspace_root(Path::new(env!("CARGO_MANIFEST_DIR")))?;
    let sh = xshell::Shell::new()?;
    sh.change_dir(&project_root);

    match cli.command {
        Commands::Build { arch } => {
            let _ = build::build_kernel(&sh, &project_root, arch)?;
        }
        Commands::Firmware { arch } => {
            firmware::build_firmware(&sh, &project_root, arch)?;
        }
        Commands::Run { arch } => {
            let kernel_elf_path = build::build_kernel(&sh, &project_root, arch)?;
            let boot_dir = build::prepare_boot_directory(&project_root, arch)?;
            let rootfs_path = build::ensure_rootfs_image(&sh, &boot_dir)?;
            let dtb_path = qemu::dump_qemu_dtb(&sh, arch, &boot_dir, &rootfs_path)?;
            let _ = qemu::generate_fit_image(
                arch,
                &sh,
                &project_root,
                &boot_dir,
                &kernel_elf_path,
                &dtb_path,
            )?;
            let _ = qemu::generate_boot_script(arch, &sh, &project_root, &boot_dir)?;
            qemu::setup_tftp(&sh, &boot_dir);
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

fn find_workspace_root(start: &Path) -> Result<PathBuf> {
    for candidate in start.ancestors() {
        let cargo_toml = candidate.join("Cargo.toml");
        if !cargo_toml.exists() {
            continue;
        }

        let content = fs::read_to_string(&cargo_toml)?;
        if content.contains("[workspace]") {
            return Ok(candidate.to_path_buf());
        }
    }

    Err(format!("failed to find workspace root from {}", start.display()).into())
}
