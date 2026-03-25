use std::fs;
use std::path::{Path, PathBuf};
use xshell::{Shell, cmd};

use crate::Result;
use crate::arch::Arch;

/// OP-TEE 32-bit Trusted Application 的编译工具链前缀。
///
/// OP-TEE 的 64-bit core 使用主工具链（`Arch::cross_compile()`），
/// 32-bit TA 需要单独的 Arm32 工具链。
const CROSS_COMPILE_ARM32: &str = "arm-linux-gnueabihf-";

/// 检查运行所需的固件文件是否就绪，缺失时返回带路径列表的错误。
pub fn ensure_firmware_exists(project_root: &Path, arch: Arch) -> Result<()> {
    let fw = arch.firmware_dir(project_root);
    let required_paths: Vec<PathBuf> = match arch {
        Arch::Riscv64 => vec![
            fw.join("u-boot/spl/u-boot-spl.bin"),
            fw.join("u-boot/u-boot.itb"),
        ],
        Arch::Aarch64 => vec![fw.join("arm-trusted-firmware/flash.bin")],
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

/// 编译指定架构所需的全部固件。
///
/// - `riscv64`: OpenSBI → U-Boot（U-Boot 依赖 OpenSBI 的 `fw_dynamic.bin`）
/// - `aarch64`: U-Boot → OP-TEE → ATF（ATF 打包 OP-TEE 和 U-Boot 为 `flash.bin`）
pub fn build_firmware(sh: &Shell, project_root: &Path, arch: Arch) -> Result<()> {
    let cross = arch.cross_compile();
    let out_base = arch.firmware_dir(project_root);
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

    // U-Boot SPL 在运行时加载 OpenSBI 作为 M-mode firmware。
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

    let cross32 = CROSS_COMPILE_ARM32;
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

    // ATF 构建产物 bl1.bin 和 fip.bin 合并为单个 flash.bin 供 QEMU 加载。
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
