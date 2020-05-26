mod ffi {
    use std::os::raw::c_char;

    #[repr(C)]
    #[derive(Clone, Copy)]
    pub struct ttfe_ui_app {
        _unused: [u8; 0],
    }

    extern "C" {
        pub fn ttfe_ui_init() -> *mut ttfe_ui_app;
        pub fn ttfe_ui_exec(app: *mut ttfe_ui_app, path: *const c_char, len: u32);
    }
}

pub struct App(*mut ffi::ttfe_ui_app);

impl App {
    pub fn new() -> Self {
        App(unsafe { ffi::ttfe_ui_init() })
    }

    pub fn exec(&mut self, path: Option<&str>) {
        match path {
            Some(path) => {
                unsafe { ffi::ttfe_ui_exec(self.0, path.as_ptr() as _, path.len() as u32) }
            }
            None => {
                unsafe { ffi::ttfe_ui_exec(self.0, std::ptr::null(), 0) }
            }
        }
    }
}
