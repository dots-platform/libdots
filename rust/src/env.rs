use std::ffi::CStr;
use std::fs::File;
use std::mem::{self,MaybeUninit};
use std::os::unix::io::FromRawFd;
use std::slice;

use crate::*;
use crate::ffi;

pub fn init() -> DotsResult<()> {
    let ret = unsafe { ffi::dots_env_init() };
    if ret != 0 {
        return Err(DotsError::from_ret(ret));
    }

    Ok(())
}

pub fn finalize() {
    unsafe {
        ffi::dots_env_finalize();
    }
}

pub fn get_world_rank() -> usize {
    unsafe { ffi::dots_env_get_world_rank() }
}

pub fn get_world_size() -> usize {
    unsafe { ffi::dots_env_get_world_size() }
}

pub fn get_in_files() -> Vec<File> {
    let in_fds: Vec<i32> = unsafe {
        let in_fds_len = ffi::dots_env_get_num_in_files();
        let mut in_fds = vec![MaybeUninit::<i32>::uninit(); in_fds_len];
        ffi::dots_env_get_in_fds(mem::transmute(in_fds.as_mut_ptr()));
        mem::transmute(in_fds)
    };

    in_fds
        .iter()
        .map(|fd| unsafe { File::from_raw_fd(*fd) })
        .collect()
}

pub fn get_out_files() -> Vec<File> {
    let out_fds: Vec<i32> = unsafe {
        let out_fds_len = ffi::dots_env_get_num_out_files();
        let mut out_fds = vec![MaybeUninit::<i32>::uninit(); out_fds_len];
        ffi::dots_env_get_out_fds(mem::transmute(out_fds.as_mut_ptr()));
        mem::transmute(out_fds)
    };

    out_fds
        .iter()
        .map(|fd| unsafe { File::from_raw_fd(*fd) })
        .collect()
}

pub fn get_func_name() -> String {
    let func_name_cstr = unsafe { CStr::from_ptr(ffi::dots_env_get_func_name()) };
    func_name_cstr
        .to_owned()
        .into_string()
        .expect("Function name is not UTF-8 encoded")
}

pub fn get_args() -> Vec<Vec<u8>> {
    let arg_ptrs: Vec<ffi::dots_env_arg_t> = unsafe {
        let args_len = ffi::dots_env_get_num_args();
        let mut arg_ptrs = vec![MaybeUninit::<ffi::dots_env_arg_t>::uninit(); args_len];
        ffi::dots_env_get_args(mem::transmute(arg_ptrs.as_mut_ptr()));
        mem::transmute(arg_ptrs)
    };

    arg_ptrs
        .iter()
        .map(|arg_ptr| unsafe { slice::from_raw_parts(arg_ptr.ptr, arg_ptr.length) }.to_owned())
        .collect()
}
