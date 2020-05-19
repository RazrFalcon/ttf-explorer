mod ffi {
    use std::os::raw::c_char;

    #[repr(C)]
    #[derive(Clone, Copy)]
    pub struct ttfe_ui_app {
        _unused: [u8; 0],
    }

    extern "C" {
        pub fn ttfe_ui_init() -> *mut ttfe_ui_app;
        pub fn ttfe_ui_exec(app: *mut ttfe_ui_app, path: *const c_char);
    }
}

pub struct App(*mut ffi::ttfe_ui_app);

impl App {
    pub fn new() -> Self {
        App(unsafe { ffi::ttfe_ui_init() })
    }

    pub fn exec(&mut self, path: Option<&str>) {
        let path_ptr = path.map(|path| path.as_ptr() as _).unwrap_or(std::ptr::null());
        unsafe { ffi::ttfe_ui_exec(self.0, path_ptr) }
    }
}
