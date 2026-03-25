use std::fs;
use std::path::{Path, PathBuf};
use xshell::{Cmd, Shell, cmd};

use crate::Result;
use crate::arch::Arch;

// boot.its.template 在编译时嵌入，避免运行时文件路径依赖。
const ITS_TEMPLATE: &str = include_str!("boot.its.template");

// 将架构相关参数填入模板，生成完整的 ITS 文件内容。
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

// 所有 QEMU 调用共享的基础参数：无图形、1 GiB 内存、2 核、
// virtio 网络/GPU/块设备、machine 和 cpu 按架构选择。
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

/// 启动 QEMU 并将硬件设备树导出到 `boot_dir/qemu.dtb`。
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

/// 将内核 ELF 和 DTB 打包为 U-Boot FIT 镜像（`boot.its` → `boot.fit`）。
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

/// 将 U-Boot 启动脚本编译为 `boot.scr.uimg`。
pub fn generate_boot_script(arch: Arch, sh: &Shell, boot_dir: &Path) -> Result<PathBuf> {
    println!("[xtask] Creating boot script image...");

    let source_script = boot_dir.join("boot_src.txt");
    fs::write(&source_script, arch.boot_script_content())?;

    let boot_scr_uimg = boot_dir.join("boot.scr.uimg");
    cmd!(sh, "mkimage -T script -d {source_script} {boot_scr_uimg}").run()?;
    Ok(boot_scr_uimg)
}

/// 在 `/srv/tftp` 下建立符号链接，供 QEMU 用户网络 TFTP 服务使用。
///
/// 需要对 `/srv/tftp` 有写权限；失败时只打警告，不中断构建流程。
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

/// 启动 QEMU 运行内核。
///
/// `debug` 为 `true` 时附加 `-s -S`：暂停 CPU 并在 `localhost:1234`
/// 开放 GDB 远程调试端口，等待 GDB 连接后再继续执行。
pub fn launch_qemu(
    sh: &Shell,
    arch: Arch,
    project_root: &Path,
    boot_dir: &Path,
    kernel_elf_path: &Path,
    rootfs_path: &Path,
    debug: bool,
) -> Result<()> {
    if debug {
        println!(
            "[xtask] Launching QEMU (debug) for {} — attach GDB on port 1234...",
            arch.as_str()
        );
    } else {
        println!("[xtask] Launching QEMU for {}...", arch.as_str());
    }

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
            let mut cmd = base_qemu_cmd(sh, arch, &rootfs_drive)
                .args(["-serial", "stdio", "-d", "guest_errors,cpu_reset"])
                .arg("-D")
                .arg(&qemu_log)
                .arg("-bios")
                .arg(&bios)
                .arg("-device")
                .arg(&loader);
            if debug {
                cmd = cmd.args(["-s", "-S"]);
            }
            cmd.run()?;
        }
        Arch::Aarch64 => {
            println!(
                "[xtask] note: connect serial consoles with `nc 127.0.0.1 54320` and `nc 127.0.0.1 54321` in separate terminals."
            );
            let bios = fw.join("arm-trusted-firmware/flash.bin");
            let fat_drive = format!("file=fat:rw:{},format=raw,media=disk", boot_dir.display());
            let mut cmd = base_qemu_cmd(sh, arch, &rootfs_drive)
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
                .arg(kernel_elf_path);
            if debug {
                cmd = cmd.args(["-s", "-S"]);
            }
            cmd.run()?;
        }
    }

    Ok(())
}
