#![windows_subsystem = "windows"]

fn main() {
    let args: Vec<_> = std::env::args().collect();
    let mut file_path = None;
    if args.len() == 2 {
        if std::path::Path::new(&args[1]).exists() {
            file_path = Some(args[1].clone());
        }
    }

    let mut app = ttf_explorer::ui::App::new();
    app.exec(file_path.as_deref());
}
