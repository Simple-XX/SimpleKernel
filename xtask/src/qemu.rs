use std::fs;
use std::path::{Path, PathBuf};
use xshell::{Cmd, Shell, cmd};

const ITS_TEMPLATE: &str = include_str!("boot.its.template");

fn generate_its_content(arch: Arch, kernel_path: &str, dtb_path: &str) -> String {
    let (fit_arch, load_addr) = match arch {
        Arch::Riscv64 => ("riscv", "0x80200000"),
        Arch::Aarch64 => ("arm64", "0x40100000"),
    };
    ITS_TEMPLATE
        .replace("{KERNEL_PATH}", kernel_path)
        .replace("{DTB_PATH}", dtb_path)
        .replace("{FIT_ARCH}", fit_arch)
        .replace("{LOAD_ADDR}", load_addr)
}

use crate::Result;
use crate::arch::Arch;

/// 所有 QEMU 调用共享的基础参数：显示、内存、外设、存储、machine/cpu。
fn base_qemu_cmd<'a>(sh: &'a Shell, arch: Arch, rootfs_drive: &str) -> Cmd<'a> {
    let (machine, cpu) = match arch {
        Arch::Riscv64 => ("virt", "max"),
        Arch::Aarch64 => ("virt,secure=on,gic_version=3", "cortex-a72"),
    };
    sh.cmd(arch.qemu_binary()).args([
        "-nographic",
        "-monitor",
        "telnet::2333,server,nowait",
        "-m",
        "1024M",
        "-smp",
        "2",
        "-global",
        "virtio-mmio.force-legacy=false",
        "-netdev",
        "user,id=net0,tftp=/srv/tftp",
        "-device",
        "virtio-net-device,netdev=net0",
        "-device",
        "virtio-gpu-device",
        "-drive",
        rootfs_drive,
        "-device",
        "virtio-blk-device,drive=hd0",
        "-machine",
        machine,
        "-cpu",
        cpu,
    ])
}

pub fn dump_qemu_dtb(
    sh: &Shell,
    arch: Arch,
    boot_dir: &Path,
    rootfs_path: &Path,
) -> Result<PathBuf> {
    let dtb_path = boot_dir.join("qemu.dtb");
    println!("[xtask] Generating QEMU DTB at {}...", dtb_path.display());

    let rootfs_drive = format!("file={},if=none,format=raw,id=hd0", rootfs_path.display());
    let dump_dtb = format!("dumpdtb={}", dtb_path.display());

    base_qemu_cmd(sh, arch, &rootfs_drive)
        .args(["-serial", "stdio", "-machine", &dump_dtb])
        .run()?;

    if !dtb_path.exists() {
        return Err(format!("failed to generate DTB at {}", dtb_path.display()).into());
    }
    Ok(dtb_path)
}

pub fn generate_fit_image(
    arch: Arch,
    sh: &Shell,
    boot_dir: &Path,
    kernel_elf_path: &Path,
    dtb_path: &Path,
) -> Result<PathBuf> {
    println!("[xtask] Generating FIT image...");

    let kernel_abs = fs::canonicalize(kernel_elf_path)?;
    let dtb_abs = fs::canonicalize(dtb_path)?;
    let content = generate_its_content(
        arch,
        &kernel_abs.display().to_string(),
        &dtb_abs.display().to_string(),
    );

    let boot_its = boot_dir.join("boot.its");
    fs::write(&boot_its, content)?;

    let boot_fit = boot_dir.join("boot.fit");
    cmd!(sh, "mkimage -f {boot_its} {boot_fit}").run()?;
    Ok(boot_fit)
}

pub fn generate_boot_script(
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

pub fn setup_tftp(boot_dir: &Path) {
    println!("[xtask] Setting up TFTP directory /srv/tftp...");

    if let Err(e) = fs::create_dir_all("/srv/tftp") {
        eprintln!(
            "[xtask] warning: failed to create /srv/tftp: {e}. You may need to run: sudo mkdir -p /srv/tftp"
        );
        return;
    }

    let boot_scr_uimg = boot_dir.join("boot.scr.uimg");
    let _ = fs::remove_file("/srv/tftp/boot.scr.uimg");
    if let Err(e) = std::os::unix::fs::symlink(&boot_scr_uimg, "/srv/tftp/boot.scr.uimg") {
        eprintln!("[xtask] warning: failed to link /srv/tftp/boot.scr.uimg: {e}");
    }

    let _ = fs::remove_file("/srv/tftp/bin");
    if let Err(e) = std::os::unix::fs::symlink(boot_dir, "/srv/tftp/bin") {
        eprintln!("[xtask] warning: failed to link /srv/tftp/bin: {e}");
    }
}

pub fn launch_qemu(
    sh: &Shell,
    arch: Arch,
    project_root: &Path,
    boot_dir: &Path,
    kernel_elf_path: &Path,
    rootfs_path: &Path,
) -> Result<()> {
    println!("[xtask] Launching QEMU for {}...", arch.as_str());

    let rootfs_drive = format!("file={},if=none,format=raw,id=hd0", rootfs_path.display());
    let qemu_log = boot_dir.join("qemu.log");
    let fw = arch.firmware_dir(project_root);

    match arch {
        Arch::Riscv64 => {
            let bios = fw.join("u-boot/spl/u-boot-spl.bin");
            let loader = format!(
                "loader,file={},addr=0x80200000",
                fw.join("u-boot/u-boot.itb").display()
            );
            base_qemu_cmd(sh, arch, &rootfs_drive)
                .args(["-serial", "stdio", "-d", "guest_errors,cpu_reset"])
                .arg("-D")
                .arg(&qemu_log)
                .arg("-bios")
                .arg(&bios)
                .arg("-device")
                .arg(&loader)
                .run()?;
        }
        Arch::Aarch64 => {
            println!(
                "[xtask] note: connect serial consoles with `nc 127.0.0.1 54320` and `nc 127.0.0.1 54321` in separate terminals."
            );
            let bios = fw.join("arm-trusted-firmware/flash.bin");
            let fat_drive = format!("file=fat:rw:{},format=raw,media=disk", boot_dir.display());
            base_qemu_cmd(sh, arch, &rootfs_drive)
                .args(["-d", "guest_errors,cpu_reset"])
                .arg("-D")
                .arg(&qemu_log)
                .arg("-drive")
                .arg(&fat_drive)
                .args([
                    "-serial",
                    "tcp:127.0.0.1:54320",
                    "-serial",
                    "tcp:127.0.0.1:54321",
                ])
                .arg("-bios")
                .arg(&bios)
                .arg("-kernel")
                .arg(kernel_elf_path)
                .run()?;
        }
    }

    Ok(())
}
