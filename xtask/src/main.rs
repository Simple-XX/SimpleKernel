fn main() {
    let mut args = std::env::args();
    let _ = args.next();

    match args.next().as_deref() {
        Some("build") => {
            println!("xtask build: stub");
        }
        Some("run") => {
            println!("xtask run: stub");
        }
        _ => {
            eprintln!("usage: cargo xtask <build|run>");
            std::process::exit(2);
        }
    }
}
