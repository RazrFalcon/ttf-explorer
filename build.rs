#[cfg(target_os = "linux")]
fn main() {
    let mut build = cc::Build::new();
    build.cpp(true);
    build.flag("-std=c++17");
    build.files(&[
        "ui/hexview.cpp",
        "ui/mainwindow.cpp",
        "ui/treemodel.cpp",
        "ui/ttfcorepp.cpp",
        "ui/ui.cpp",
    ]);
    build.include("ui");

    let lib = pkg_config::find_library("Qt5Widgets").expect("Unable to find Qt5Widgets");
    for path in lib.include_paths {
        build.include(path.to_str().unwrap());
    }

    build.compile("libui.a");
}

#[cfg(target_os = "windows")]
fn main() {
    let qt_dir = std::env::var("QT_DIR").expect("QT_DIR is not set");
    let qt_path = std::path::Path::new(&qt_dir);

    let mut build = cc::Build::new();
    let tool = build.get_compiler();

    build.cpp(true);
    build.files(&[
        "ui/hexview.cpp",
        "ui/mainwindow.cpp",
        "ui/treemodel.cpp",
        "ui/ttfcorepp.cpp",
        "ui/ui.cpp",
    ]);
    build.include("ui");

    build.include(qt_path.join("include"));
    build.include(qt_path.join("include").join("QtCore"));
    build.include(qt_path.join("include").join("QtGui"));
    build.include(qt_path.join("include").join("QtWidgets"));

    if tool.is_like_msvc() {
        build.compile("libqtc.lib");
    } else {
        build.flag("-std=c++17");
        build.compile("libui.a");
    }

    println!("cargo:rustc-link-search={}", qt_path.join("bin").display()); // for MinGW
    println!("cargo:rustc-link-search={}", qt_path.join("lib").display()); // for MSVC

    println!("cargo:rustc-link-lib=Qt5Core");
    println!("cargo:rustc-link-lib=Qt5Gui");
    println!("cargo:rustc-link-lib=Qt5Widgets");
}

#[cfg(target_os = "macos")]
fn main() {
    let qt_dir = std::env::var("QT_DIR").expect("QT_DIR is not set");
    let qt_path = std::path::Path::new(&qt_dir);

    let mut build = cc::Build::new();
    build.cpp(true);
    build.flag("-std=c++17");
    build.flag(&format!("-F{}/lib", qt_dir));
    build.files(&[
        "ui/hexview.cpp",
        "ui/mainwindow.cpp",
        "ui/treemodel.cpp",
        "ui/ttfcorepp.cpp",
        "ui/ui.cpp",
    ]);
    build.include("ui");

    build.include(qt_path.join("lib/QtCore.framework/Headers"));
    build.include(qt_path.join("lib/QtGui.framework/Headers"));
    build.include(qt_path.join("lib/QtWidgets.framework/Headers"));

    build.compile("libui.a");

    println!("cargo:rustc-link-search=framework={}/lib", qt_dir);
    println!("cargo:rustc-link-lib=framework=QtCore");
    println!("cargo:rustc-link-lib=framework=QtGui");
    println!("cargo:rustc-link-lib=framework=QtWidgets");
}
