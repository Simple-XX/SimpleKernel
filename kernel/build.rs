use std::path::PathBuf;

fn main() {
    let arch = std::env::var("CARGO_CFG_TARGET_ARCH").unwrap();
    let arch_dir = PathBuf::from("src/arch").join(&arch);

    let asm_files: Vec<&str> = match arch.as_str() {
        "riscv64" => vec!["boot.S", "switch.S", "interrupt.S", "macro.S"],
        "aarch64" => vec!["boot.S", "switch.S", "interrupt.S", "macro.S"],
        _ => panic!("unsupported architecture: {arch}"),
    };

    let compiler = match arch.as_str() {
        "riscv64" => "riscv64-linux-gnu-gcc",
        "aarch64" => "aarch64-linux-gnu-gcc",
        _ => unreachable!(),
    };

    let mut build = cc::Build::new();
    build.compiler(compiler);

    // Preprocessor defines expected by assembly files
    // (must match CMakePresets.json cache variables)
    build.define("SIMPLEKERNEL_MAX_CORE_COUNT", "4");
    build.define("SIMPLEKERNEL_DEFAULT_STACK_SIZE", "16384");
    if arch == "riscv64" {
        build.define("SIMPLEKERNEL_EARLY_CONSOLE_BASE", "0x10000000");
        build.define("SIMPLEKERNEL_PER_CPU_ALIGN_SIZE", "128");
        build.flag("-march=rv64gc");
        build.flag("-mabi=lp64d");
    } else {
        build.define("SIMPLEKERNEL_EARLY_CONSOLE_BASE", "0x9000000");
        build.define("SIMPLEKERNEL_PER_CPU_ALIGN_SIZE", "128");
    }

    build.include(&arch_dir);

    for file in &asm_files {
        build.file(arch_dir.join(file));
    }
    build.compile("asm");

    // Linker arguments: disable RELRO (not applicable to bare-metal) and set linker script
    println!("cargo:rustc-link-arg=-z");
    println!("cargo:rustc-link-arg=norelro");
    println!(
        "cargo:rustc-link-arg=-T{}",
        arch_dir.join("link.ld").display()
    );

    // Rebuild triggers
    println!("cargo:rerun-if-changed=src/arch/{}/link.ld", arch);
    for file in &asm_files {
        println!("cargo:rerun-if-changed=src/arch/{}/{}", arch, file);
    }
}
