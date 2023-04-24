use std::ffi::CStr;

use crate::ffi as ffi;

pub fn init() -> Result<(), &'static str> {
    unsafe {
        if ffi::dots_env_init() != 0 {
            return Err("dots_env_init");
        }
    }
    Ok(())
}

pub fn finalize() {
    unsafe {
        ffi::dots_env_finalize();
    }
}

pub fn get_world_rank() -> usize {
    unsafe {
        ffi::dots_env_get_world_rank()
    }
}

pub fn get_world_size() -> usize {
    unsafe {
        ffi::dots_env_get_world_size()
    }
}

pub fn get_in_fds() -> Vec<i32> {
    unsafe {
        let in_fds_len = ffi::dots_env_get_num_in_files();
        let mut in_fds = Vec::with_capacity(in_fds_len);
        ffi::dots_env_get_in_fds(in_fds.as_mut_ptr());
        in_fds.set_len(in_fds_len);
        in_fds
    }
}

pub fn get_out_fds() -> Vec<i32> {
    unsafe {
        let out_fds_len = ffi::dots_env_get_num_out_files();
        let mut out_fds = Vec::with_capacity(out_fds_len);
        ffi::dots_env_get_out_fds(out_fds.as_mut_ptr());
        out_fds.set_len(out_fds_len);
        out_fds
    }
}

pub fn get_func_name() -> String {
    unsafe {
        CStr::from_ptr(ffi::dots_env_get_func_name())
            .to_owned()
            .into_string()
            .expect("Function name is not UTF-8 encoded")
    }
}
