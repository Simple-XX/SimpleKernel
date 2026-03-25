use std::path::PathBuf;

// 内核编译期常量
const MAX_CORE_COUNT: u32 = 4;
const DEFAULT_STACK_SIZE: u32 = 16384;
const PER_CPU_ALIGN_SIZE: u32 = 128;

/// 架构相关配置。
struct ArchConfig {
    compiler: &'static str,
    early_console_base: &'static str,
    extra_flags: &'static [&'static str],
}

fn arch_config(arch: &str) -> ArchConfig {
    match arch {
        "riscv64" => ArchConfig {
            compiler: "riscv64-linux-gnu-gcc",
            early_console_base: "0x10000000",
            extra_flags: &["-march=rv64gc", "-mabi=lp64d"],
        },
        "aarch64" => ArchConfig {
            compiler: "aarch64-linux-gnu-gcc",
            early_console_base: "0x9000000",
            extra_flags: &[],
        },
        _ => unreachable!("unsupported architecture: {arch}"),
    }
}

fn main() {
    let arch = std::env::var("CARGO_CFG_TARGET_ARCH").expect("CARGO_CFG_TARGET_ARCH not set");
    let os = std::env::var("CARGO_CFG_TARGET_OS").unwrap_or_default();

    if !matches!(arch.as_str(), "riscv64" | "aarch64") || os != "none" {
        return;
    }

    let arch_dir = PathBuf::from("src/arch").join(&arch);
    let cfg = arch_config(&arch);

    // ── 编译汇编文件 ──────────────────────────────────────────────────
    let mut build = cc::Build::new();
    build.compiler(cfg.compiler);

    build.define("SIMPLEKERNEL_MAX_CORE_COUNT", &*MAX_CORE_COUNT.to_string());
    build.define(
        "SIMPLEKERNEL_DEFAULT_STACK_SIZE",
        &*DEFAULT_STACK_SIZE.to_string(),
    );
    build.define("SIMPLEKERNEL_EARLY_CONSOLE_BASE", cfg.early_console_base);
    build.define(
        "SIMPLEKERNEL_PER_CPU_ALIGN_SIZE",
        &*PER_CPU_ALIGN_SIZE.to_string(),
    );

    for flag in cfg.extra_flags {
        build.flag(flag);
    }

    build.include(&arch_dir);

    // 自动发现 .S 文件，新增汇编源文件无需手动注册。
    for entry in std::fs::read_dir(&arch_dir)
        .unwrap_or_else(|e| panic!("cannot read {}: {e}", arch_dir.display()))
    {
        let path = entry
            .unwrap_or_else(|e| panic!("cannot read entry in {}: {e}", arch_dir.display()))
            .path();
        if path.extension().is_some_and(|ext| ext == "S") {
            build.file(&path);
        }
    }

    build.compile("asm");

    // ── 链接器参数 ───────────────────────────────────────────────────
    println!("cargo:rustc-link-arg=-z");
    println!("cargo:rustc-link-arg=norelro");
    println!(
        "cargo:rustc-link-arg=-T{}",
        arch_dir.join("link.ld").display()
    );

    // 监视整个架构目录，头文件/链接脚本/新增 .S 的变更都会触发重建。
    println!("cargo:rerun-if-changed={}", arch_dir.display());
}
