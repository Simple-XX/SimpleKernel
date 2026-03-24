use clap::{Parser, Subcommand, ValueEnum};
use std::fs;
use std::path::{Path, PathBuf};
use std::process;
use xshell::{Shell, cmd};

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

#[derive(Clone, Copy, Debug, ValueEnum)]
enum Arch {
    Riscv64,
    Aarch64,
}

impl Arch {
    fn as_str(self) -> &'static str {
        match self {
            Self::Riscv64 => "riscv64",
            Self::Aarch64 => "aarch64",
        }
    }

    fn target_json(self) -> &'static str {
        match self {
            Self::Riscv64 => "riscv64gc-simplekernel.json",
            Self::Aarch64 => "aarch64-simplekernel.json",
        }
    }

    fn target_triple(self) -> &'static str {
        match self {
            Self::Riscv64 => "riscv64gc-simplekernel",
            Self::Aarch64 => "aarch64-simplekernel",
        }
    }

    fn qemu_binary(self) -> &'static str {
        match self {
            Self::Riscv64 => "qemu-system-riscv64",
            Self::Aarch64 => "qemu-system-aarch64",
        }
    }

    fn its_template(self) -> &'static str {
        match self {
            Self::Riscv64 => "riscv64_qemu_virt.its.in",
            Self::Aarch64 => "aarch64_qemu_virt.its.in",
        }
    }

    fn boot_script_template(self) -> &'static str {
        match self {
            Self::Riscv64 => "riscv64_boot_scr.txt",
            Self::Aarch64 => "aarch64_boot_scr.txt",
        }
    }

    fn cross_compile(self) -> &'static str {
        match self {
            Self::Riscv64 => "riscv64-linux-gnu-",
            Self::Aarch64 => "aarch64-linux-gnu-",
        }
    }

    fn firmware_dir(self) -> &'static str {
        match self {
            Self::Riscv64 => "build_riscv64",
            Self::Aarch64 => "build_aarch64",
        }
    }
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
    let sh = Shell::new()?;
    sh.change_dir(&project_root);

    match cli.command {
        Commands::Build { arch } => {
            let _ = build_kernel(&sh, &project_root, arch)?;
        }
        Commands::Firmware { arch } => {
            build_firmware(&sh, &project_root, arch)?;
        }
        Commands::Run { arch } => {
            let kernel_elf_path = build_kernel(&sh, &project_root, arch)?;
            let boot_dir = prepare_boot_directory(&project_root, arch)?;
            let rootfs_path = ensure_rootfs_image(&sh, &boot_dir)?;
            let dtb_path = dump_qemu_dtb(&sh, arch, &boot_dir, &rootfs_path)?;
            let _ = generate_fit_image(
                arch,
                &sh,
                &project_root,
                &boot_dir,
                &kernel_elf_path,
                &dtb_path,
            )?;
            let _ = generate_boot_script(arch, &sh, &project_root, &boot_dir)?;
            setup_tftp(&sh, &boot_dir);
            ensure_firmware_exists(&project_root, arch)?;
            launch_qemu(
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

fn build_kernel(sh: &Shell, project_root: &Path, arch: Arch) -> Result<PathBuf> {
    println!("[xtask] Building kernel for {}...", arch.as_str());
    let target_json = format!("targets/{}", arch.target_json());
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

fn prepare_boot_directory(project_root: &Path, arch: Arch) -> Result<PathBuf> {
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

fn ensure_rootfs_image(sh: &Shell, boot_dir: &Path) -> Result<PathBuf> {
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

fn dump_qemu_dtb(sh: &Shell, arch: Arch, boot_dir: &Path, rootfs_path: &Path) -> Result<PathBuf> {
    let dtb_path = boot_dir.join("qemu.dtb");
    println!("[xtask] Generating QEMU DTB at {}...", dtb_path.display());

    let qemu = arch.qemu_binary();
    let rootfs_drive = format!("file={},if=none,format=raw,id=hd0", rootfs_path.display());
    let dump_arg = format!("dumpdtb={}", dtb_path.display());
    match arch {
        Arch::Riscv64 => {
            cmd!(
                sh,
                "{qemu} -nographic -serial stdio -monitor telnet::2333,server,nowait -m 1024M -smp 2 -global virtio-mmio.force-legacy=false -netdev user,id=net0,tftp=/srv/tftp -device virtio-net-device,netdev=net0 -device virtio-gpu-device -drive {rootfs_drive} -device virtio-blk-device,drive=hd0 -machine virt -cpu max -machine {dump_arg}"
            )
            .run()?;
        }
        Arch::Aarch64 => {
            cmd!(
                sh,
                "{qemu} -nographic -serial stdio -monitor telnet::2333,server,nowait -m 1024M -smp 2 -global virtio-mmio.force-legacy=false -netdev user,id=net0,tftp=/srv/tftp -device virtio-net-device,netdev=net0 -device virtio-gpu-device -drive {rootfs_drive} -device virtio-blk-device,drive=hd0 -machine virt,secure=on,gic_version=3 -cpu cortex-a72 -machine {dump_arg}"
            )
            .run()?;
        }
    }

    if !dtb_path.exists() {
        return Err(format!("failed to generate DTB at {}", dtb_path.display()).into());
    }
    Ok(dtb_path)
}

fn generate_fit_image(
    arch: Arch,
    sh: &Shell,
    project_root: &Path,
    boot_dir: &Path,
    kernel_elf_path: &Path,
    dtb_path: &Path,
) -> Result<PathBuf> {
    println!("[xtask] Generating FIT image...");

    let template_path = project_root.join("tools").join(arch.its_template());
    let template = fs::read_to_string(&template_path)?;
    let kernel_abs = fs::canonicalize(kernel_elf_path)?;
    let dtb_abs = fs::canonicalize(dtb_path)?;

    let content = template
        .replace("@DESC@", "simplekernel")
        .replace("@KERNEL_PATH@", &kernel_abs.display().to_string())
        .replace("@DTB_PATH@", &dtb_abs.display().to_string());

    let boot_its = boot_dir.join("boot.its");
    fs::write(&boot_its, content)?;

    let boot_fit = boot_dir.join("boot.fit");
    cmd!(sh, "mkimage -f {boot_its} {boot_fit}").run()?;
    Ok(boot_fit)
}

fn generate_boot_script(
    arch: Arch,
    sh: &Shell,
    project_root: &Path,
    boot_dir: &Path,
) -> Result<PathBuf> {
    println!("[xtask] Creating boot script image...");

    let source_script = project_root.join("tools").join(arch.boot_script_template());
    if !source_script.exists() {
        return Err(format!("boot script not found at {}", source_script.display()).into());
    }

    let boot_scr_uimg = boot_dir.join("boot.scr.uimg");
    cmd!(sh, "mkimage -T script -d {source_script} {boot_scr_uimg}").run()?;
    Ok(boot_scr_uimg)
}

fn setup_tftp(sh: &Shell, boot_dir: &Path) {
    println!("[xtask] Setting up TFTP directory /srv/tftp...");

    if let Err(error) = cmd!(sh, "mkdir -p /srv/tftp").run() {
        eprintln!(
            "[xtask] warning: failed to create /srv/tftp: {error}. You may need to create it manually: sudo mkdir -p /srv/tftp"
        );
    }

    let boot_scr_uimg = boot_dir.join("boot.scr.uimg");
    let _ = fs::remove_file("/srv/tftp/boot.scr.uimg");
    if let Err(error) = cmd!(sh, "ln -sf {boot_scr_uimg} /srv/tftp/boot.scr.uimg").run() {
        eprintln!("[xtask] warning: failed to link /srv/tftp/boot.scr.uimg: {error}");
    }

    let _ = fs::remove_file("/srv/tftp/bin");
    if let Err(error) = cmd!(sh, "ln -sf {boot_dir} /srv/tftp/bin").run() {
        eprintln!("[xtask] warning: failed to link /srv/tftp/bin: {error}");
    }
}

fn ensure_firmware_exists(project_root: &Path, arch: Arch) -> Result<()> {
    let required_paths: Vec<PathBuf> = match arch {
        Arch::Riscv64 => vec![
            firmware_output_dir(project_root, arch).join("u-boot/spl/u-boot-spl.bin"),
            firmware_output_dir(project_root, arch).join("u-boot/u-boot.itb"),
        ],
        Arch::Aarch64 => {
            vec![firmware_output_dir(project_root, arch).join("arm-trusted-firmware/flash.bin")]
        }
    };

    let missing: Vec<PathBuf> = required_paths
        .into_iter()
        .filter(|path| !path.exists())
        .collect();
    if missing.is_empty() {
        return Ok(());
    }

    let arch_str = arch.as_str();
    let mut message =
        format!("Firmware not found. Run 'cargo xtask firmware --arch {arch_str}' first.");
    for path in missing {
        message.push_str("\n  missing: ");
        message.push_str(&path.display().to_string());
    }
    Err(message.into())
}

fn firmware_output_dir(project_root: &Path, arch: Arch) -> PathBuf {
    project_root.join(arch.firmware_dir()).join("3rd")
}

fn build_firmware(sh: &Shell, project_root: &Path, arch: Arch) -> Result<()> {
    let cross = arch.cross_compile();
    let out_base = project_root.join(arch.firmware_dir()).join("3rd");
    let jobs = std::thread::available_parallelism()
        .map(|n| n.get().to_string())
        .unwrap_or_else(|_| "4".into());

    match arch {
        Arch::Riscv64 => {
            build_opensbi(sh, project_root, &out_base, cross, &jobs)?;
            build_uboot_riscv64(sh, project_root, &out_base, cross, &jobs)?;
        }
        Arch::Aarch64 => {
            build_uboot_aarch64(sh, project_root, &out_base, cross, &jobs)?;
            build_optee(sh, project_root, &out_base, cross, &jobs)?;
            build_atf(sh, project_root, &out_base, cross, &jobs)?;
        }
    }

    println!("[xtask] Firmware build complete for {}.", arch.as_str());
    Ok(())
}

fn build_opensbi(
    sh: &Shell,
    project_root: &Path,
    out_base: &Path,
    cross: &str,
    jobs: &str,
) -> Result<()> {
    println!("[xtask] Building OpenSBI...");
    let src = project_root.join("3rd/opensbi");
    let out = out_base.join("opensbi");
    fs::create_dir_all(&out)?;

    let _dir = sh.push_dir(&src);
    cmd!(
        sh,
        "make PLATFORM_RISCV_XLEN=64 PLATFORM=generic FW_JUMP_ADDR=0x80210000 FW_OPTIONS=0 CROSS_COMPILE={cross} O={out} -j{jobs}"
    )
    .run()?;
    Ok(())
}

fn build_uboot_riscv64(
    sh: &Shell,
    project_root: &Path,
    out_base: &Path,
    cross: &str,
    jobs: &str,
) -> Result<()> {
    println!("[xtask] Building U-Boot (riscv64)...");
    let src = project_root.join("3rd/u-boot");
    let out = out_base.join("u-boot");
    fs::create_dir_all(&out)?;

    let opensbi_fw = out_base.join("opensbi/platform/generic/firmware/fw_dynamic.bin");
    if !opensbi_fw.exists() {
        return Err(format!(
            "OpenSBI firmware not found at {}. Build OpenSBI first.",
            opensbi_fw.display()
        )
        .into());
    }

    let _dir = sh.push_dir(&src);
    cmd!(sh, "make O={out} qemu-riscv64_spl_defconfig -j{jobs}").run()?;
    cmd!(
        sh,
        "make CROSS_COMPILE={cross} O={out} OPENSBI={opensbi_fw} -j{jobs}"
    )
    .run()?;
    Ok(())
}

fn build_uboot_aarch64(
    sh: &Shell,
    project_root: &Path,
    out_base: &Path,
    cross: &str,
    jobs: &str,
) -> Result<()> {
    println!("[xtask] Building U-Boot (aarch64)...");
    let src = project_root.join("3rd/u-boot");
    let out = out_base.join("u-boot");
    fs::create_dir_all(&out)?;

    let _dir = sh.push_dir(&src);
    cmd!(sh, "make O={out} qemu_arm64_defconfig -j{jobs}").run()?;
    cmd!(sh, "make CROSS_COMPILE={cross} O={out} -j{jobs}").run()?;
    Ok(())
}

fn build_optee(
    sh: &Shell,
    project_root: &Path,
    out_base: &Path,
    cross: &str,
    jobs: &str,
) -> Result<()> {
    println!("[xtask] Building OP-TEE...");
    let src = project_root.join("3rd/optee/optee_os");
    let out = out_base.join("optee/optee_os");
    fs::create_dir_all(&out)?;

    let cross32 = "arm-linux-gnueabihf-";

    let _dir = sh.push_dir(&src);
    cmd!(
        sh,
        "make CFG_ARM64_core=y CFG_TEE_BENCHMARK=n CFG_TEE_CORE_LOG_LEVEL=3 CROSS_COMPILE={cross} CROSS_COMPILE_core={cross} CROSS_COMPILE_ta_arm32={cross32} CROSS_COMPILE_ta_arm64={cross} DEBUG=0 O={out} PLATFORM=vexpress-qemu_armv8a CFG_ARM_GICV3=y -j{jobs}"
    )
    .run()?;
    Ok(())
}

fn build_atf(
    sh: &Shell,
    project_root: &Path,
    out_base: &Path,
    cross: &str,
    jobs: &str,
) -> Result<()> {
    println!("[xtask] Building ARM Trusted Firmware...");
    let src = project_root.join("3rd/arm-trusted-firmware");
    let atf_out = out_base.join("arm-trusted-firmware");
    fs::create_dir_all(&atf_out)?;

    let optee_out = out_base.join("optee/optee_os");
    let bl32 = optee_out.join("core/tee-header_v2.bin");
    let bl32_extra1 = optee_out.join("core/tee-pager_v2.bin");
    let bl32_extra2 = optee_out.join("core/tee-pageable_v2.bin");
    let bl33 = out_base.join("u-boot/u-boot.bin");

    for path in [&bl32, &bl32_extra1, &bl32_extra2, &bl33] {
        if !path.exists() {
            return Err(format!("dependency not found: {}", path.display()).into());
        }
    }

    let _dir = sh.push_dir(&src);
    cmd!(
        sh,
        "make DEBUG=0 CROSS_COMPILE={cross} PLAT=qemu BUILD_BASE={atf_out} BL32={bl32} BL32_EXTRA1={bl32_extra1} BL32_EXTRA2={bl32_extra2} BL33={bl33} BL32_RAM_LOCATION=tdram QEMU_USE_GIC_DRIVER=QEMU_GICV3 SPD=opteed all fip -j{jobs}"
    )
    .run()?;

    let flash_bin = atf_out.join("flash.bin");
    let bl1 = atf_out.join("qemu/release/bl1.bin");
    let fip = atf_out.join("qemu/release/fip.bin");

    cmd!(sh, "dd if={bl1} of={flash_bin} bs=4096 conv=notrunc").run()?;
    cmd!(
        sh,
        "dd if={fip} of={flash_bin} seek=64 bs=4096 conv=notrunc"
    )
    .run()?;

    println!("[xtask] ATF flash.bin created at {}", flash_bin.display());
    Ok(())
}

fn launch_qemu(
    sh: &Shell,
    arch: Arch,
    project_root: &Path,
    boot_dir: &Path,
    kernel_elf_path: &Path,
    rootfs_path: &Path,
) -> Result<()> {
    println!("[xtask] Launching QEMU for {}...", arch.as_str());

    let qemu = arch.qemu_binary();
    let qemu_log_path = boot_dir.join("qemu.log");
    let rootfs_drive = format!("file={},if=none,format=raw,id=hd0", rootfs_path.display());

    let fw = firmware_output_dir(project_root, arch);

    match arch {
        Arch::Riscv64 => {
            let bios_path = fw.join("u-boot/spl/u-boot-spl.bin");
            let loader_path = fw.join("u-boot/u-boot.itb");
            let loader_arg = format!("loader,file={},addr=0x80200000", loader_path.display());
            cmd!(
                sh,
                "{qemu} -nographic -serial stdio -monitor telnet::2333,server,nowait -m 1024M -smp 2 -d guest_errors,cpu_reset -global virtio-mmio.force-legacy=false -netdev user,id=net0,tftp=/srv/tftp -device virtio-net-device,netdev=net0 -device virtio-gpu-device -machine virt -cpu max -drive {rootfs_drive} -device virtio-blk-device,drive=hd0 -D {qemu_log_path} -bios {bios_path} -device {loader_arg}"
            )
            .run()?;
        }
        Arch::Aarch64 => {
            println!(
                "[xtask] note: connect serial consoles with `nc 127.0.0.1 54320` and `nc 127.0.0.1 54321` in separate terminals."
            );
            let bios_path = fw.join("arm-trusted-firmware/flash.bin");
            let boot_fat_drive =
                format!("file=fat:rw:{},format=raw,media=disk", boot_dir.display());
            cmd!(
                sh,
                "{qemu} -nographic -monitor telnet::2333,server,nowait -m 1024M -smp 2 -d guest_errors,cpu_reset -global virtio-mmio.force-legacy=false -netdev user,id=net0,tftp=/srv/tftp -device virtio-net-device,netdev=net0 -device virtio-gpu-device -machine virt,secure=on,gic_version=3 -cpu cortex-a72 -drive {rootfs_drive} -device virtio-blk-device,drive=hd0 -D {qemu_log_path} -drive {boot_fat_drive} -serial tcp:127.0.0.1:54320 -serial tcp:127.0.0.1:54321 -bios {bios_path} -kernel {kernel_elf_path}"
            )
            .run()?;
        }
    }

    Ok(())
}
