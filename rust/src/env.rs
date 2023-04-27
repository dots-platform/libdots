use std::ffi::CStr;
use std::fs::File;
use std::mem::MaybeUninit;
use std::os::unix::io::FromRawFd;
use std::slice;

use crate::*;
use crate::ffi as ffi;

pub fn init() -> DotsResult<()> {
    unsafe {
        let ret = ffi::dots_env_init();
        if ret != 0 {
            return Err(DotsError::from_ret(ret));
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

pub fn get_in_files() -> Vec<File> {
    let in_fds = unsafe {
        let in_fds_len = ffi::dots_env_get_num_in_files();
        let mut in_fds = Vec::with_capacity(in_fds_len);
        ffi::dots_env_get_in_fds(in_fds.as_mut_ptr());
        in_fds.set_len(in_fds_len);
        in_fds
    };

    let mut in_files = Vec::with_capacity(in_fds.len());
    for fd in in_fds {
        let in_file = unsafe { File::from_raw_fd(fd) };
        in_files.push(in_file)
    };
    in_files
}

pub fn get_out_files() -> Vec<File> {
    let out_fds = unsafe {
        let out_fds_len = ffi::dots_env_get_num_out_files();
        let mut out_fds = Vec::with_capacity(out_fds_len);
        ffi::dots_env_get_out_fds(out_fds.as_mut_ptr());
        out_fds.set_len(out_fds_len);
        out_fds
    };

    let mut out_files = Vec::with_capacity(out_fds.len());
    for fd in out_fds {
        let out_file = unsafe { File::from_raw_fd(fd) };
        out_files.push(out_file)
    };
    out_files
}

pub fn get_func_name() -> String {
    unsafe {
        CStr::from_ptr(ffi::dots_env_get_func_name())
            .to_owned()
            .into_string()
            .expect("Function name is not UTF-8 encoded")
    }
}

pub fn get_args() -> Vec<Vec<u8>> {
    unsafe {
        let args_len = ffi::dots_env_get_num_args();
        let mut arg_ptrs: Vec<ffi::dots_env_arg_t> = vec![MaybeUninit::uninit().assume_init(); args_len];
        ffi::dots_env_get_args(arg_ptrs.as_mut_ptr());
        arg_ptrs
            .iter()
            .map(|arg_ptr| slice::from_raw_parts(arg_ptr.ptr, arg_ptr.length).to_owned())
            .collect()
    }
}
